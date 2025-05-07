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

// Configurations nécessaires pour vTaskList
#define configUSE_TRACE_FACILITY 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1

#include "../../include/core/task_manager.h"
#include "utils/logging.h"
#include "communication/wifi_manager.h"
#include "../../include/communication/wifi_manager.h"
#include <ElegantOTA.h>
#include "hardware/io/display_manager.h"
#include "hardware/io/potentiometer_manager.h"
#include "hardware/io/button_ui.h"
#include "hardware/sensors/imu.h"
#include "hardware/sensors/line_length.h"
#include "hardware/sensors/tension.h"
#include "hardware/sensors/wind.h"
#include "hardware/actuators/servo.h"

/* === MODULE TASK MANAGER ===
   Implémentation du gestionnaire de tâches FreeRTOS pour le système Kite PiloteV3.
   Ce module gère la création, la surveillance et l'arrêt des tâches.
*/

#include "../../include/core/task_manager.h"
#include "../../include/utils/logging.h"

// === VARIABLES GLOBALES ===
static TaskDefinition tasks[MAX_TASKS]; // Tableau des tâches gérées
static int taskCount = 0;              // Nombre de tâches ajoutées

// Déclaration des variables statiques de la classe TaskManager
UIManager* TaskManager::uiManager = nullptr;
WiFiManager* TaskManager::wifiManager = nullptr;
QueueHandle_t TaskManager::messageQueue = nullptr;
SemaphoreHandle_t TaskManager::displayMutex = nullptr;
TaskHandle_t TaskManager::displayTaskHandle = nullptr;  // Initialisation du handle de tâche d'affichage
TaskHandle_t TaskManager::buttonTaskHandle = nullptr;   // Initialisation du handle de tâche des boutons

// Handle pour la tâche de monitoring
TaskHandle_t monitorTaskHandle = nullptr;

// === FONCTIONS ===

/**
 * Initialise le gestionnaire de tâches.
 */
void taskManagerInit() {
  taskCount = 0;
  LOG_INFO("TASK_MANAGER", "Gestionnaire de tâches initialisé");
}

/**
 * Ajoute une tâche au gestionnaire.
 * @param task Définition de la tâche à ajouter.
 * @return true si la tâche a été ajoutée avec succès, false sinon.
 */
bool taskManagerAddTask(TaskDefinition task) {
  if (taskCount >= MAX_TASKS) {
    LOG_ERROR("TASK_MANAGER", "Nombre maximum de tâches atteint");
    return false;
  }

  tasks[taskCount++] = task;
  LOG_INFO("TASK_MANAGER", "Tâche ajoutée : %s", task.name);
  return true;
}

/**
 * Démarre toutes les tâches gérées.
 */
void taskManagerStartTasks() {
  for (int i = 0; i < taskCount; i++) {
    TaskDefinition* task = &tasks[i];
    xTaskCreate(task->taskFunction, task->name, task->stackSize, NULL, task->priority, &task->handle);
    LOG_INFO("TASK_MANAGER", "Tâche démarrée : %s", task->name);
  }
}

/**
 * Arrête toutes les tâches gérées.
 */
void taskManagerStopTasks() {
  for (int i = 0; i < taskCount; i++) {
    TaskDefinition* task = &tasks[i];
    if (task->handle != NULL) {
      vTaskDelete(task->handle);
      LOG_INFO("TASK_MANAGER", "Tâche arrêtée : %s", task->name);
      task->handle = NULL;
    }
  }
}

/**
 * Affiche l'état des tâches gérées.
 */
void taskManagerPrintStatus() {
  LOG_INFO("TASK_MANAGER", "État des tâches :");
  for (int i = 0; i < taskCount; i++) {
    TaskDefinition* task = &tasks[i];
    LOG_INFO("TASK_MANAGER", "- Tâche : %s, Priorité : %d", task->name, task->priority);
  }
}

/**
 * Initialise le gestionnaire de tâches avec les gestionnaires associés.
 * @param uiManager Gestionnaire d'interface utilisateur.
 * @param wifiManager Gestionnaire WiFi.
 * @return true si l'initialisation a réussi, false sinon.
 */
bool TaskManager::begin(UIManager* uiManager, WiFiManager* wifiManager) {
  // Initialisation des pointeurs statiques vers les gestionnaires
  TaskManager::uiManager = uiManager;
  TaskManager::wifiManager = wifiManager;
  
  // Initialisation des ressources partagées
  messageQueue = xQueueCreate(10, sizeof(Message));
  displayMutex = xSemaphoreCreateMutex();
  
  if (!messageQueue || !displayMutex) {
    LOG_ERROR("TASK_MANAGER", "Échec de la création des ressources partagées");
    return false;
  }
  
  running = true;
  LOG_INFO("TASK_MANAGER", "Gestionnaire de tâches initialisé");
  return true;
}

/**
 * Constructeur du gestionnaire de tâches
 */
TaskManager::TaskManager() {
    running = false;
    tasksRunning = false;
    lastTaskMetricsTime = 0;
    
    // Initialisation des handles
    for (int i = 0; i < MAX_TASKS; i++) {
        taskHandles[i] = nullptr;
        taskParams[i] = nullptr;
    }
}

/**
 * Destructeur du gestionnaire de tâches
 */
TaskManager::~TaskManager() {
    stopAllTasks();
}

/**
 * Démarre toutes les tâches gérées
 * @return true si succès, false si échec
 */
bool TaskManager::startTasks() {
    if (!running) {
        LOG_ERROR("TASK_MANAGER", "Le gestionnaire de tâches n'est pas initialisé");
        return false;
    }
    
    if (tasksRunning) {
        LOG_WARNING("TASK_MANAGER", "Les tâches sont déjà en cours d'exécution");
        return true;
    }
    
    // Démarrage des tâches principales 
    BaseType_t result;
    
    // Tâche d'affichage avec priorité inférieure
    result = xTaskCreate(
        displayTask,
        "Display",
        DISPLAY_TASK_STACK_SIZE,
        nullptr,
        1,  // Priorité basse (1-24, 24 étant la plus élevée)
        &displayTaskHandle
    );
    
    if (result != pdPASS) {
        LOG_ERROR("TASK_MANAGER", "Échec de création de la tâche d'affichage");
        return false;
    }
    
    // Ajout d'un petit délai entre les créations de tâches
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Tâche des boutons avec priorité moyenne
    result = xTaskCreate(
        buttonTask,
        "Buttons", 
        POT_TASK_STACK_SIZE,
        nullptr,
        2,  // Priorité moyenne
        &buttonTaskHandle
    );
    
    if (result != pdPASS) {
        LOG_ERROR("TASK_MANAGER", "Échec de création de la tâche des boutons");
        // Nettoyer la tâche précédemment créée
        if (displayTaskHandle != nullptr) {
            vTaskDelete(displayTaskHandle);
            displayTaskHandle = nullptr;
        }
        return false;
    }
    
    // Ajout d'un petit délai entre les créations de tâches
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Tâche de monitoring avec priorité basse
    result = xTaskCreate(
        monitorTask,
        "Monitor",
        MONITOR_TASK_STACK_SIZE,
        nullptr,
        1,  // Priorité basse
        &monitorTaskHandle
    );
    
    if (result != pdPASS) {
        LOG_ERROR("TASK_MANAGER", "Échec de création de la tâche de monitoring");
        // Nettoyer les tâches précédemment créées
        if (displayTaskHandle != nullptr) {
            vTaskDelete(displayTaskHandle);
            displayTaskHandle = nullptr;
        }
        if (buttonTaskHandle != nullptr) {
            vTaskDelete(buttonTaskHandle);
            buttonTaskHandle = nullptr;
        }
        return false;
    }
    
    // Ajout d'un petit délai pour la synchronisation des logs
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // D'autres tâches peuvent être ajoutées ici au besoin
    
    tasksRunning = true;
    LOG_INFO("TASK_MANAGER", "Toutes les tâches ont été démarrées");
    
    // Ajout d'un petit délai pour éviter les chevauchements de logs
    vTaskDelay(pdMS_TO_TICKS(10));
    
    return true;
}

/**
 * Arrête toutes les tâches gérées
 */
void TaskManager::stopTasks() {
    if (!tasksRunning) {
        return;
    }
    
    // Arrêt des tâches
    if (displayTaskHandle != nullptr) {
        vTaskDelete(displayTaskHandle);
        displayTaskHandle = nullptr;
    }
    
    if (buttonTaskHandle != nullptr) {
        vTaskDelete(buttonTaskHandle);
        buttonTaskHandle = nullptr;
    }
    
    if (monitorTaskHandle != nullptr) {
        vTaskDelete(monitorTaskHandle);
        monitorTaskHandle = nullptr;
    }
    
    // Arrêt des autres tâches
    
    tasksRunning = false;
    LOG_INFO("TASK_MANAGER", "Toutes les tâches ont été arrêtées");
}

/**
 * Arrête toutes les tâches et libère les ressources
 */
void TaskManager::stopAllTasks() {
    stopTasks();
    
    // Libération des ressources partagées
    if (messageQueue != nullptr) {
        vQueueDelete(messageQueue);
        messageQueue = nullptr;
    }
    
    if (displayMutex != nullptr) {
        vSemaphoreDelete(displayMutex);
        displayMutex = nullptr;
    }
    
    running = false;
    LOG_INFO("TASK_MANAGER", "Toutes les ressources ont été libérées");
}

/**
 * Fonction pour la tâche d'affichage - mise à jour périodique de l'écran LCD
 */
void TaskManager::displayTask(void* parameters) {
    extern DisplayManager display;   // Référence au gestionnaire d'affichage global
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    LOG_INFO("TASK_LCD", "Tâche d'affichage démarrée");
    
    for (;;) {
        // Mise à jour périodique de l'écran toutes les 500 ms
        display.updateMainDisplay();
        
        // Utilisation de vTaskDelayUntil pour une temporisation précise
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(500));
    }
}

/**
 * Fonction pour la tâche des boutons - lecture périodique de l'état des boutons
 */
void TaskManager::buttonTask(void* parameters) {
    extern ButtonUIManager buttonUI;  // Référence à l'interface de boutons globale
    extern PotentiometerManager potManager;  // Référence au gestionnaire de potentiomètres global
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    LOG_INFO("TASK_BTN", "Tâche des boutons démarrée");
    
    for (;;) {
        // Lecture de l'état des boutons - utiliser la méthode existante
        buttonUI.readButton(BUTTON_UP);
        buttonUI.readButton(BUTTON_DOWN);
        buttonUI.readButton(BUTTON_SELECT);
        buttonUI.readButton(BUTTON_BACK);
        
        // Lecture des valeurs des potentiomètres individuellement
        int direction = potManager.getDirection();
        int trim = potManager.getTrim();
        int lineLength = potManager.getLineLength();
        
        // Convertir les valeurs des potentiomètres (-100..100) aux plages des servomoteurs
        // Direction: -90..90 degrés
        // Trim: -45..45 degrés
        // LineLength: 0..100%
        int servoDirection = map(direction, -100, 100, DIRECTION_MIN_ANGLE, DIRECTION_MAX_ANGLE);
        int servoTrim = map(trim, -100, 100, TRIM_MIN_ANGLE, TRIM_MAX_ANGLE);
        int servoLineModulation = map(lineLength, 0, 100, 0, 100);
        
        // Mettre à jour les servomoteurs avec les valeurs des potentiomètres
        servoUpdateAll(servoDirection, servoTrim, servoLineModulation);
        
        // Log pour déboguer (seulement toutes les 20 itérations pour ne pas saturer)
        static int logCounter = 0;
        if (++logCounter >= 20) {
            LOG_DEBUG("SERVO_CTRL", "Mise à jour servos: Dir=%d, Trim=%d, Line=%d", 
                     servoDirection, servoTrim, servoLineModulation);
            logCounter = 0;
        }
        
        // Utilisation de vTaskDelayUntil pour une temporisation précise
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(50));
    }
}

/**
 * Fonction pour la tâche réseau
 */
void TaskManager::networkTask(void* parameters) {
    for (;;) {
        // Code de la tâche réseau
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/**
 * Fonction pour la tâche de contrôle
 */
void TaskManager::controlTask(void* parameters) {
    for (;;) {
        // Code de la tâche de contrôle
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/**
 * Fonction pour la tâche d'entrée
 */
void TaskManager::inputTask(void* parameters) {
    for (;;) {
        // Code de la tâche d'entrée
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * Fonction pour la tâche de surveillance
 */
void TaskManager::monitorTask(void* parameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    unsigned long counter = 0;
    
    LOG_INFO("TASK_MON", "Tâche de monitoring démarrée");
    
    for (;;) {
        // Envoi de logs périodiques pour vérifier que la communication série fonctionne
        counter++;
        LOG_INFO("MONITOR", "Surveillance système active (cycle #%lu)", counter);
        
        // Vérification de l'état des tâches principales
        if (displayTaskHandle != nullptr) {
            LOG_DEBUG("MONITOR", "Tâche d'affichage: active");
        }
        
        if (buttonTaskHandle != nullptr) {
            LOG_DEBUG("MONITOR", "Tâche des boutons: active");
        }
        
        // Journalisation de l'utilisation mémoire
        logMemoryUsage("MONITOR");
        
        // Toutes les 10 itérations, afficher un graphique mémoire
        if (counter % 10 == 0) {
            logMemoryGraph();
        }
        
        // Utilisation de vTaskDelayUntil pour une temporisation précise (5 secondes)
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(5000));
    }
}

/**
 * Fonction pour la tâche des capteurs
 */
void TaskManager::sensorTask(void* parameters) {
    for (;;) {
        // Code de la tâche des capteurs
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * Met à jour les métriques des tâches
 */
void TaskManager::updateTaskMetrics() {
    // Code pour mettre à jour les métriques des tâches
}

/**
 * Vérifie l'état de santé des tâches
 * @return true si toutes les tâches sont en bonne santé, false sinon
 */
bool TaskManager::checkTasksHealth() {
    // Code pour vérifier l'état de santé des tâches
    return true;
}
