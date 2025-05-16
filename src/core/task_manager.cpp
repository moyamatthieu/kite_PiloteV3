/*
  -----------------------
  Kite PiloteV3 - Module de Gestion des Tâches (Implémentation)
  -----------------------
  
  Implémentation du gestionnaire de tâches FreeRTOS pour le système Kite PiloteV3.
  
  Version: 3.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
  
  ===== FONCTIONNEMENT =====
  Ce module orchestre l'exécution des différentes tâches du système en utilisant
  le framework FreeRTOS. Il assure la coordination et la synchronisation entre
  les différentes fonctionnalités du cerf-volant.
  
  Principes de fonctionnement :
  1. Création et gestion des tâches pour les différents sous-systèmes
  2. Allocation des priorités et ressources pour optimiser les performances
  3. Coordination des échanges de messages entre les tâches
  4. Surveillance de l'état de santé des tâches et récupération en cas de panne
  
  Interactions avec d'autres modules :
  - UIManager : Interface utilisateur locale (LCD, boutons)
  - WiFiManager : Communication sans fil
  - Display : Gestion de l'affichage LCD
  - ButtonUI : Gestion des entrées utilisateur
  - Servos : Contrôle des actionneurs
  - Sensors : Acquisition de données des capteurs
  
  Aspects techniques notables :
  - Utilisation du modèle producteur-consommateur via des files de messages
  - Protection des ressources partagées avec des mutex
  - Optimisation des rafraîchissements LCD avec buffer matriciel pour éviter le scintillement
  - Surveillance des temps d'exécution des tâches pour détecter les blocages
*/

// Configurations nécessaires pour vTaskList et uxTaskGetStackHighWaterMark
#define configUSE_TRACE_FACILITY 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1 // Nécessaire pour vTaskList, peut être optionnel sinon

#include "core/task_manager.h"
#include "core/logging.h" // Pour LOG_INFO, LOG_ERROR, etc.
#include "core/component.h"
#include "common/global_enums.h"
#include "common/ipc_message.h" // Ajout de l'inclusion pour IPCMessage
#include "core/config.h" // Pour les configurations de tâches (stack, prio)

// Initialisation des membres statiques
QueueHandle_t TaskManager::systemMessageQueue = nullptr;
SemaphoreHandle_t TaskManager::displayMutex = nullptr;
TaskHandle_t TaskManager::systemMonitorTaskHandle = nullptr;

// === IMPLÉMENTATION DE TASKMANAGER ===

TaskManager::TaskManager() : running(false), tasksRunning(false) {
    LOG_INFO("TaskManager", "Gestionnaire de tâches créé.");
}

TaskManager::~TaskManager() {
    stopAll();
    LOG_INFO("TaskManager", "Gestionnaire de tâches détruit.");
}

bool TaskManager::begin(const std::vector<ManagedComponent*>& managedComponents) {
    if (running) {
        LOG_WARNING("TaskManager", "Le gestionnaire de tâches est déjà initialisé.");
        return true;
    }

    this->components = managedComponents;

    // Création des ressources partagées
    systemMessageQueue = xQueueCreate(SYSTEM_MESSAGE_QUEUE_SIZE, sizeof(IPCMessage)); // IPCMessage vient de common/ipc_message.h
    if (systemMessageQueue == nullptr) {
        LOG_ERROR("TaskManager", "Échec de création de la file de messages système.");
        return false;
    }

    displayMutex = xSemaphoreCreateMutex();
    if (displayMutex == nullptr) {
        LOG_ERROR("TaskManager", "Échec de création du mutex d'affichage.");
        vQueueDelete(systemMessageQueue);
        systemMessageQueue = nullptr;
        return false;
    }
    
    // Initialiser les composants enregistrés
    for (ManagedComponent* component : this->components) {
        if (component) {
            // Passer les ressources nécessaires aux composants si besoin
        }
    }

    running = true;
    LOG_INFO("TaskManager", "Gestionnaire de tâches initialisé avec %d composants.", this->components.size());
    return true;
}

bool TaskManager::startManagedTasks() {
    if (!running) {
        LOG_ERROR("TaskManager", "Gestionnaire non initialisé. Appelez begin() d'abord.");
        return false;
    }
    if (tasksRunning) {
        LOG_WARNING("TaskManager", "Les tâches des composants sont déjà en cours d'exécution.");
        return true;
    }

    LOG_INFO("TaskManager", "Démarrage des tâches des composants...");
    for (ManagedComponent* component : components) {
        if (component && component->isEnabled() && !component->isTaskRunning()) { // Vérifier si activé et pas déjà en cours
            // Tenter d'initialiser le composant s'il ne l'est pas déjà.
            // Cela suppose que l'initialisation est idempotente ou gère son propre état.
            if (!component->isInitialized()) {
                LOG_INFO("TaskManager", "Initialisation du composant : %s", component->getComponentName());
                if (component->initialize() != ErrorCode::OK) {
                    LOG_ERROR("TaskManager", "Échec de l'initialisation du composant : %s. Tâche non démarrée.", component->getComponentName());
                    component->disable(); // Désactiver le composant au lieu d'utiliser setState directement
                    continue; // Passer au composant suivant
                }
            }

            // Démarrer la tâche uniquement si le composant est maintenant initialisé (IDLE ou ACTIVE)
            if (component->getState() == ComponentState::IDLE || component->getState() == ComponentState::ACTIVE) {
                LOG_INFO("TaskManager", "Démarrage de la tâche pour le composant : %s", component->getComponentName());
                if (!component->startTask()) {
                    LOG_ERROR("TaskManager", "Échec du démarrage de la tâche pour le composant : %s", component->getComponentName());
                    component->disable(); // Désactiver le composant au lieu d'utiliser setState directement
                } else {
                    // Optionnel: Mettre à jour l'état si startTask réussit et que le composant doit passer à ACTIVE
                    // Cela dépend de la logique de startTask de chaque composant.
                    // Si startTask met déjà le composant dans l'état ACTIVE, ceci n'est pas nécessaire.
                    // component->setState(ComponentState::ACTIVE); 
                }
            } else {
                 LOG_WARNING("TaskManager", "Composant %s non prêt après initialisation (état: %s), tâche non démarrée.", component->getComponentName(), component->stateString());
            }
        } else if (component && !component->isEnabled()) {
            LOG_INFO("TaskManager", "Composant %s désactivé, tâche non démarrée.", component->getComponentName());
        } else if (component && component->isTaskRunning()) {
            LOG_INFO("TaskManager", "Tâche pour le composant %s déjà en cours.", component->getComponentName());
        }
    }

    // Démarrer la tâche de monitoring système
    BaseType_t result = xTaskCreate(
        systemMonitorTask,
        "SysMonitor",
        SYSTEM_MONITOR_STACK_SIZE, // À définir dans config.h
        this, // Passer l'instance de TaskManager si nécessaire, sinon nullptr
        SYSTEM_MONITOR_PRIORITY,   // À définir dans config.h
        &systemMonitorTaskHandle
    );

    if (result != pdPASS) {
        LOG_ERROR("TaskManager", "Échec de création de la tâche de monitoring système.");
    } else {
        LOG_INFO("TaskManager", "Tâche de monitoring système démarrée.");
    }

    tasksRunning = true;
    LOG_INFO("TaskManager", "Toutes les tâches des composants (applicables) et le monitoring ont été démarrés.");
    return true;
}

void TaskManager::stopManagedTasks() {
    if (!tasksRunning) {
        LOG_INFO("TaskManager", "Les tâches des composants ne sont pas en cours d'exécution.");
        return;
    }

    LOG_INFO("TaskManager", "Arrêt des tâches des composants...");
    for (ManagedComponent* component : components) {
        if (component && component->isTaskRunning()) { // Vérifier si la tâche est en cours avant d'arrêter
            LOG_INFO("TaskManager", "Arrêt de la tâche pour le composant : %s", component->getComponentName());
            component->stopTask();
        }
    }

    if (systemMonitorTaskHandle != nullptr) {
        LOG_INFO("TaskManager", "Arrêt de la tâche de monitoring système.");
        vTaskDelete(systemMonitorTaskHandle);
        systemMonitorTaskHandle = nullptr;
    }

    tasksRunning = false;
    LOG_INFO("TaskManager", "Toutes les tâches des composants et le monitoring ont été arrêtés.");
}

void TaskManager::stopAll() {
    LOG_INFO("TaskManager", "Arrêt complet du TaskManager...");
    stopManagedTasks();

    if (systemMessageQueue != nullptr) {
        vQueueDelete(systemMessageQueue);
        systemMessageQueue = nullptr;
        LOG_INFO("TaskManager", "File de messages système supprimée.");
    }

    if (displayMutex != nullptr) {
        vSemaphoreDelete(displayMutex);
        displayMutex = nullptr;
        LOG_INFO("TaskManager", "Mutex d'affichage supprimé.");
    }
    
    components.clear();

    running = false;
    LOG_INFO("TaskManager", "TaskManager complètement arrêté et ressources libérées.");
}

// Tâche de monitoring système
void TaskManager::systemMonitorTask(void* parameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t monitorInterval = pdMS_TO_TICKS(SYSTEM_MONITOR_INTERVAL_MS); // Définir dans config.h

    // Utilisez des fonctions basiques d'Arduino plutôt que LOG_INFO
    // car à ce stade de l'exécution, LoggingModule pourrait ne pas être initialisé
    Serial.println("[SysMonitor] Tâche de monitoring système démarrée.");

    for (;;) {
        vTaskDelayUntil(&lastWakeTime, monitorInterval);
    }
}

QueueHandle_t TaskManager::getSystemMessageQueue() {
    return systemMessageQueue;
}

SemaphoreHandle_t TaskManager::getDisplayMutex() {
    return displayMutex;
}
