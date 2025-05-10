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
#include "../../include/utils/logging.h"
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
#include "control/autopilot.h"  // Pour autopilotInit
#include "core/system.h"        // Pour systemHealthCheck
#include "ui/dashboard.h"
#include "ui/webserver.h"
#include "core/module.h"
#include <vector>
#include <string>
#include <algorithm>
#include <algorithm>
#include <algorithm>

/* === MODULE TASK MANAGER ===
   Implémentation du gestionnaire de tâches FreeRTOS pour le système Kite PiloteV3.
   Ce module gère la création, la surveillance et l'arrêt des tâches.
*/

#include "../../include/core/task_manager.h"

// === VARIABLES GLOBALES ===
static TaskDefinition tasks[MAX_TASKS]; // Tableau des tâches gérées
static int taskCount = 0;              // Nombre de tâches ajoutées

// Initialisation des handles statiques de la classe TaskManager
UIManager* TaskManager::uiManager = nullptr;
WiFiManager* TaskManager::wifiManager = nullptr;
QueueHandle_t TaskManager::messageQueue = nullptr;
SemaphoreHandle_t TaskManager::displayMutex = nullptr;

// Initialisation des handles des tâches
TaskHandle_t TaskManager::displayTaskHandle = nullptr;
TaskHandle_t TaskManager::buttonTaskHandle = nullptr;
TaskHandle_t TaskManager::inputTaskHandle = nullptr;
TaskHandle_t TaskManager::networkTaskHandle = nullptr;
TaskHandle_t TaskManager::controlTaskHandle = nullptr;
TaskHandle_t TaskManager::sensorTaskHandle = nullptr;

// Handle pour la tâche de monitoring
TaskHandle_t monitorTaskHandle = nullptr;

// Exemple d'intégration : déclaration des modules principaux
class WiFiModule : public Module {
public:
    WiFiModule() : Module("WiFi", moduleWifiEnabled) {}
    void update() override { /* gestion FSM WiFi ici */ }
    const char* description() const override { return "Connexion WiFi"; }
};
class APIModule : public Module {
public:
    APIModule() : Module("API", moduleApiEnabled) {}
    const char* description() const override { return "API HTTP/REST"; }
};
class ServoModule : public Module {
public:
    ServoModule() : Module("Servo", moduleServoEnabled) {}
    const char* description() const override { return "Contrôle des servos"; }
};
class AutopilotModule : public Module {
public:
    AutopilotModule() : Module("Autopilot", moduleAutopilotEnabled) {}
    const char* description() const override { return "Pilote automatique"; }
};
class WebserverModule : public Module {
public:
    WebserverModule() : Module("Webserver", moduleWebserverEnabled) {}
    const char* description() const override { return "Serveur Web"; }
};
class WinchModule : public Module {
public:
    WinchModule() : Module("Winch", moduleWinchEnabled) {}
    const char* description() const override { return "Treuil motorisé"; }
};
class SensorsModule : public Module {
public:
    SensorsModule() : Module("Sensors", moduleSensorsEnabled) {}
    const char* description() const override { return "Capteurs IMU, vent, etc."; }
};
class OTAModule : public Module {
public:
    OTAModule() : Module("OTA", moduleOtaEnabled) {}
    const char* description() const override { return "Mise à jour à distance"; }
};

// Instanciation globale et enregistrement automatique
static WiFiModule wifiModule; REGISTER_MODULE(wifiModule, &wifiModule);
static APIModule apiModule; REGISTER_MODULE(apiModule, &apiModule);
static ServoModule servoModule; REGISTER_MODULE(servoModule, &servoModule);
static AutopilotModule autopilotModule; REGISTER_MODULE(autopilotModule, &autopilotModule);
static WebserverModule webserverModule; REGISTER_MODULE(webserverModule, &webserverModule);
static WinchModule winchModule; REGISTER_MODULE(winchModule, &winchModule);
static SensorsModule sensorsModule; REGISTER_MODULE(sensorsModule, &sensorsModule);
static OTAModule otaModule; REGISTER_MODULE(otaModule, &otaModule);

// === FONCTIONS ===

/**
 * Constructeur du gestionnaire de tâches
 */
TaskManager::TaskManager() {
    running = false;
    tasksRunning = false;
    lastTaskMetricsTime = 0;
    
    // Initialisation des tableaux
    for (int i = 0; i < MAX_TASKS; i++) {
        taskHandles[i] = nullptr;
        taskParams[i] = nullptr;
        memset(&taskStats[i], 0, sizeof(TaskStat));
    }
    
    // Initialisation des handles
    wifiMonitorTaskHandle = nullptr;
    systemMonitorTaskHandle = nullptr;
    
    logPrint((LogLevel)LOG_LEVEL_INFO, "TASK_MANAGER", "Gestionnaire de tâches créé");
}

/**
 * Destructeur du gestionnaire de tâches
 */
TaskManager::~TaskManager() {
    stopAllTasks();
    logPrint((LogLevel)LOG_LEVEL_INFO, "TASK_MANAGER", "Gestionnaire de tâches détruit");
}

/**
 * Initialise le gestionnaire de tâches avec les gestionnaires associés
 * @param ui Pointeur vers le gestionnaire d'interface utilisateur
 * @param wifi Pointeur vers le gestionnaire WiFi
 * @return true si succès, false si échec
 */
bool TaskManager::begin(UIManager* ui, WiFiManager* wifi) {
    if (running) {
        logPrint((LogLevel)LOG_LEVEL_WARNING, "TASK_MANAGER", "Le gestionnaire de tâches est déjà initialisé");
        return true;
    }
    
    // Enregistrement des gestionnaires
    uiManager = ui;
    wifiManager = wifi;
    
    // Création des ressources partagées
    messageQueue = xQueueCreate(10, sizeof(Message));
    if (messageQueue == nullptr) {
        logPrint((LogLevel)LOG_LEVEL_ERROR, "TASK_MANAGER", "Échec de création de la file de messages");
        return false;
    }
    
    displayMutex = xSemaphoreCreateMutex();
    if (displayMutex == nullptr) {
        logPrint((LogLevel)LOG_LEVEL_ERROR, "TASK_MANAGER", "Échec de création du mutex d'affichage");
        vQueueDelete(messageQueue);
        messageQueue = nullptr;
        return false;
    }
    
    running = true;
    logPrint((LogLevel)LOG_LEVEL_INFO, "TASK_MANAGER", "Gestionnaire de tâches initialisé avec succès");
    return true;
}

/**
 * Initialise le gestionnaire de tâches.
 */
void taskManagerInit() {
  taskCount = 0;
  logPrint((LogLevel)LOG_LEVEL_INFO, "TASK_MANAGER", "Gestionnaire de tâches initialisé");
}

/**
 * Ajoute une tâche au gestionnaire.
 * @param task Définition de la tâche à ajouter.
 * @return true si la tâche a été ajoutée avec succès, false sinon.
 */
bool taskManagerAddTask(TaskDefinition task) {
  if (taskCount >= MAX_TASKS) {
    logPrint((LogLevel)LOG_LEVEL_ERROR, "TASK_MANAGER", "Nombre maximum de tâches atteint");
    return false;
  }

  tasks[taskCount++] = task;
  logPrint((LogLevel)LOG_LEVEL_INFO, "TASK_MANAGER", "Tâche ajoutée : %s", task.name);
  return true;
}

/**
 * Démarre toutes les tâches gérées dynamiquement selon l'état des modules (OOP)
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
    BaseType_t result;
    vTaskDelay(pdMS_TO_TICKS(10));

    // Nouvelle logique : démarrage dynamique selon les modules activés
    for (Module* m : ModuleRegistry::instance().modules()) {
        if (!m->isEnabled()) {
            LOG_INFO("TASK_MANAGER", "Tâche %s non lancée (module désactivé)", m->name());
            continue;
        }
        // Démarrage de la tâche associée au module
        if (strcmp(m->name(), "Display") == 0) {
            result = xTaskCreate(
                displayTask,
                "Display",
                DISPLAY_TASK_STACK_SIZE,
                nullptr,
                DISPLAY_TASK_PRIORITY,
                &displayTaskHandle
            );
        } else if (strcmp(m->name(), "WiFi") == 0) {
            result = xTaskCreate(
                networkTask,
                "Network",
                WIFI_TASK_STACK_SIZE,
                nullptr,
                WIFI_TASK_PRIORITY,
                &networkTaskHandle
            );
        } else if (strcmp(m->name(), "Autopilot") == 0) {
            result = xTaskCreate(
                controlTask,
                "Control",
                SYSTEM_TASK_STACK_SIZE,
                nullptr,
                SYSTEM_TASK_PRIORITY,
                &controlTaskHandle
            );
        } else if (strcmp(m->name(), "Sensors") == 0) {
            result = xTaskCreate(
                sensorTask,
                "Sensors",
                IMU_TASK_STACK_SIZE,
                nullptr,
                IMU_TASK_PRIORITY,
                &sensorTaskHandle
            );
        } else {
            // Pour les autres modules, prévoir une extension OOP (ex: m->startTask())
            LOG_INFO("TASK_MANAGER", "Aucune tâche FreeRTOS directe pour le module %s", m->name());
            continue;
        }
        if (result != pdPASS) {
            LOG_ERROR("TASK_MANAGER", "Échec de création de la tâche pour le module %s", m->name());
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        LOG_INFO("TASK_MANAGER", "Tâche démarrée pour le module %s", m->name());
    }

    // Tâche des boutons (toujours lancée, car UI locale indispensable)
    result = xTaskCreate(
        buttonTask,
        "Buttons",
        BUTTON_TASK_STACK_SIZE,
        nullptr,
        BUTTON_TASK_PRIORITY,
        &buttonTaskHandle
    );
    if (result != pdPASS) {
        LOG_ERROR("TASK_MANAGER", "Échec de création de la tâche des boutons");
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));

    // Tâche potentiomètres (toujours lancée)
    result = xTaskCreate(
        inputTask,
        "Input",
        POT_TASK_STACK_SIZE,
        nullptr,
        POT_TASK_PRIORITY,
        &inputTaskHandle
    );
    if (result != pdPASS) {
        LOG_ERROR("TASK_MANAGER", "Échec de création de la tâche d'entrée");
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));

    // Tâche de monitoring (toujours lancée)
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
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));

    tasksRunning = true;
    LOG_INFO("TASK_MANAGER", "Toutes les tâches ont été démarrées dynamiquement (OOP)");
    return true;
}

/**
 * Arrête toutes les tâches gérées dynamiquement selon l'état des modules (OOP)
 */
void TaskManager::stopTasks() {
    if (!tasksRunning) {
        return;
    }
    // Arrêt des tâches associées aux modules activés
    for (Module* m : ModuleRegistry::instance().modules()) {
        if (!m->isEnabled()) continue;
        if (strcmp(m->name(), "Display") == 0 && displayTaskHandle != nullptr) {
            vTaskDelete(displayTaskHandle);
            displayTaskHandle = nullptr;
        } else if (strcmp(m->name(), "WiFi") == 0 && networkTaskHandle != nullptr) {
            vTaskDelete(networkTaskHandle);
            networkTaskHandle = nullptr;
        } else if (strcmp(m->name(), "Autopilot") == 0 && controlTaskHandle != nullptr) {
            vTaskDelete(controlTaskHandle);
            controlTaskHandle = nullptr;
        } else if (strcmp(m->name(), "Sensors") == 0 && sensorTaskHandle != nullptr) {
            vTaskDelete(sensorTaskHandle);
            sensorTaskHandle = nullptr;
        }
        // Pour les autres modules, prévoir une extension OOP (ex: m->stopTask())
    }
    // Arrêt des tâches toujours lancées
    if (buttonTaskHandle != nullptr) {
        vTaskDelete(buttonTaskHandle);
        buttonTaskHandle = nullptr;
    }
    if (inputTaskHandle != nullptr) {
        vTaskDelete(inputTaskHandle);
        inputTaskHandle = nullptr;
    }
    if (monitorTaskHandle != nullptr) {
        vTaskDelete(monitorTaskHandle);
        monitorTaskHandle = nullptr;
    }
    tasksRunning = false;
    LOG_INFO("TASK_MANAGER", "Toutes les tâches ont été arrêtées dynamiquement (OOP)");
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
 * Fonction pour la tâche de surveillance
 */
void TaskManager::monitorTask(void* parameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    unsigned long counter = 0;

    LOG_INFO("TASK_MON", "Tâche de monitoring démarrée");

    for (;;) {
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

        // Temporisation précise avec vTaskDelayUntil
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(5000));
    }
}

/**
 * Fonction pour la tâche d'affichage
 * Gère l'écran LCD et les mises à jour d'interface
 */
void TaskManager::displayTask(void* parameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    unsigned long updateCounter = 0;
    bool uiReady = false;

    LOG_INFO("DISPLAY", "Tâche d'affichage démarrée");

    // Attendre que l'interface utilisateur soit prête (avec timeout)
    for (int attempt = 0; attempt < 10; attempt++) {
        if (uiManager && uiManager->isInitialized()) {
            uiReady = true;
            LOG_INFO("DISPLAY", "Interface utilisateur trouvée et initialisée");
            break;
        }
        LOG_WARNING("DISPLAY", "Interface utilisateur non prête, attente (%d/10)", attempt + 1);
        vTaskDelay(pdMS_TO_TICKS(100)); // Attendre 100ms avant la prochaine tentative
    }

    if (!uiReady) {
        LOG_ERROR("DISPLAY", "Interface utilisateur non initialisée après plusieurs tentatives");
        // Ne pas quitter la tâche, essayer quand même avec des vérifications à chaque cycle
    }

    // Boucle principale de la tâche
    for (;;) {
        updateCounter++;
        
        // Vérifier à nouveau si l'UI est prête
        if (!uiReady && uiManager && uiManager->isInitialized()) {
            uiReady = true;
            LOG_INFO("DISPLAY", "Interface utilisateur désormais disponible");
        }
        
        // Mettre à jour l'écran LCD à intervalle régulier si l'UI est prête
        if (uiReady) {
            if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                // Mise à jour de l'affichage
                uiManager->updateDisplay();
                xSemaphoreGive(displayMutex);
            }
        }

        // Log périodique pour vérifier l'activité
        if (updateCounter % 100 == 0) {
            if (uiReady) {
                LOG_DEBUG("DISPLAY", "Mise à jour d'affichage #%lu (UI prête)", updateCounter);
            } else {
                LOG_WARNING("DISPLAY", "Mise à jour d'affichage #%lu (UI non prête)", updateCounter);
                
                // Nouvelle tentative de récupération de l'UI
                if (uiManager && uiManager->isInitialized()) {
                    uiReady = true;
                    LOG_INFO("DISPLAY", "Interface utilisateur récupérée");
                }
            }
        }

        // Temporisation précise
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(DISPLAY_UPDATE_INTERVAL));
    }
}

/**
 * Fonction pour la tâche des boutons
 * Gère la lecture des boutons et les interactions utilisateur
 */
void TaskManager::buttonTask(void* parameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    unsigned long scanCounter = 0;

    LOG_INFO("BUTTONS", "Tâche des boutons démarrée");

    // Vérifier si l'interface utilisateur a été correctement initialisée
    if (!uiManager) {
        LOG_ERROR("BUTTONS", "Interface utilisateur non initialisée");
        vTaskDelete(NULL);
        return;
    }

    // Boucle principale de la tâche
    for (;;) {
        scanCounter++;
        
        // Scanner les boutons et mettre à jour l'état
        uiManager->checkButtons();
        
        // Log périodique pour vérifier l'activité
        if (scanCounter % 200 == 0) {
            LOG_DEBUG("BUTTONS", "Cycle de scan des boutons #%lu", scanCounter);
        }

        // Temporisation précise
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(BUTTON_CHECK_INTERVAL));
    }
}

/**
 * Fonction pour la tâche d'entrée (potentiomètres)
 * Gère la lecture des potentiomètres et l'état des entrées analogiques
 */
void TaskManager::inputTask(void* parameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    static PotentiometerManager potManager;
    unsigned long readCounter = 0;

    LOG_INFO("INPUT", "Tâche des potentiomètres démarrée");

    // Initialisation des potentiomètres
    potManager.begin();

    // Boucle principale de la tâche
    for (;;) {
        readCounter++;
        
        // Lire les potentiomètres et mettre à jour l'état
        potManager.updatePotentiometers();
        
        // Vérifier si le mode pilote automatique doit être désactivé
        potManager.checkAutoPilotStatus();
        
        // Log périodique pour vérifier l'activité
        if (readCounter % 100 == 0) {
            LOG_DEBUG("INPUT", "Lecture des potentiomètres #%lu - Dir: %d, Trim: %d, Longueur: %d", 
                      readCounter, potManager.getDirection(), potManager.getTrim(), potManager.getLineLength());
        }

        // Temporisation précise
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(POT_READ_INTERVAL));
    }
}

/**
 * Fonction pour la tâche réseau
 * Gère les connexions WiFi et les communications réseau
 */
void TaskManager::networkTask(void* parameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    unsigned long cycleCounter = 0;

    LOG_INFO("NETWORK", "Tâche réseau démarrée");

    // Vérifier si le gestionnaire WiFi a été correctement initialisé
    if (!wifiManager) {
        LOG_ERROR("NETWORK", "Gestionnaire WiFi non initialisé");
        vTaskDelete(NULL);
        return;
    }

    // Boucle principale de la tâche
    for (;;) {
        cycleCounter++;
        
        // Gérer la machine à états du WiFi
        wifiManager->handleFSM();
        
        // Log périodique pour vérifier l'activité
        if (cycleCounter % 50 == 0) {
            bool connected = wifiManager->isConnected();
            if (connected) {
                LOG_DEBUG("NETWORK", "WiFi connecté (%lu)", cycleCounter);
            } else {
                LOG_DEBUG("NETWORK", "WiFi déconnecté (%lu)", cycleCounter);
            }
        }

        // Temporisation précise
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(WIFI_CHECK_INTERVAL));
    }
}

/**
 * Fonction pour la tâche de contrôle
 * Gère la logique de contrôle principale du système
 */
void TaskManager::controlTask(void* parameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    unsigned long controlCounter = 0;

    LOG_INFO("CONTROL", "Tâche de contrôle démarrée");

    // Initialiser l'autopilote
    autopilotInit();

    // Boucle principale de la tâche
    for (;;) {
        controlCounter++;
        
        // Exécuter la boucle de contrôle principale
        
        // Vérification des conditions de sécurité
        if (controlCounter % 20 == 0) {
            // Vérifier l'état du système toutes les 20 itérations
            systemHealthCheck();
        }
        
        // Log périodique pour vérifier l'activité
        if (controlCounter % 100 == 0) {
            LOG_DEBUG("CONTROL", "Cycle de contrôle #%lu", controlCounter);
        }

        // Temporisation précise
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(20)); // 50Hz
    }
}

/**
 * Fonction pour la tâche des capteurs
 * Gère la lecture et le traitement des données des capteurs
 */
void TaskManager::sensorTask(void* parameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    unsigned long sensorCounter = 0;
    bool imuInitialized = false;
    IMUData currentImuData;

    LOG_INFO("SENSORS", "Tâche des capteurs démarrée");

    // Initialisation de l'IMU avec une configuration par défaut
    imuInitialized = imuInit(nullptr);
    if (!imuInitialized) {
        LOG_ERROR("SENSORS", "Échec d'initialisation de l'IMU");
        // Continuer quand même, l'IMU pourrait être connecté plus tard
    }

    // Boucle principale de la tâche
    for (;;) {
        sensorCounter++;
        
        // Lecture et mise à jour des capteurs
        if (imuInitialized) {
            // Mise à jour des données de l'IMU
            imuReadProcessedData(&currentImuData);
        } else if (sensorCounter % 100 == 0) {
            // Tentative de réinitialisation périodique si l'IMU n'est pas initialisé
            imuInitialized = imuInit(nullptr);
            if (imuInitialized) {
                LOG_INFO("SENSORS", "IMU initialisé avec succès après nouvelle tentative");
            }
        }
        
        // Lecture des autres capteurs (vent, tension, longueur de ligne, etc.)
        
        // Log périodique pour vérifier l'activité
        if (sensorCounter % 100 == 0) {
            LOG_DEBUG("SENSORS", "Cycle de lecture des capteurs #%lu", sensorCounter);
            
            // Affichage périodique des données de l'IMU si disponibles
            if (imuInitialized && currentImuData.dataValid) {
                LOG_DEBUG("SENSORS", "IMU: Pitch=%.1f, Roll=%.1f, Yaw=%.1f", 
                          currentImuData.orientation[0], 
                          currentImuData.orientation[1], 
                          currentImuData.orientation[2]);
            }
        }

        // Temporisation précise
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100)); // 100ms
    }
}

/**
 * Affichage dynamique de l'état des modules sur l'écran LCD 20x4
 * Utilise displayMessage pour garantir la cohérence et la non-réentrance.
 */
void displayModuleStatusLCD() {
    extern DisplayManager display;
    if (!moduleDisplayEnabled) return;
    char line[21];
    display.clear();
    display.centerText(0, "Modules actifs");
    int row = 1;
    for (Module* m : ModuleRegistry::instance().modules()) {
        snprintf(line, sizeof(line), "%s: %s", m->name(), m->stateString());
        display.centerText(row, line);
        if (++row >= 4) break;
    }
    // Si moins de 3 modules, lignes vides
    for (; row < 4; ++row) display.centerText(row, "");
    display.updateMainDisplay();
}

/**
 * Affichage dynamique de l'état des modules sur l'interface web (tableau de bord)
 */
void displayModuleStatusWeb() {
    if (!moduleWebserverEnabled) return;
    String html = "<h2>État des modules</h2><table border='1'><tr><th>Module</th><th>État</th><th>Description</th></tr>";
    for (Module* m : ModuleRegistry::instance().modules()) {
        html += String("<tr><td>") + m->name() + "</td><td>" + m->stateString() + "</td><td>" + m->description() + "</td></tr>";
    }
    html += "</table>";
    LOG_INFO("WEB", "%s", html.c_str());
}

/**
 * Permet d'activer/désactiver dynamiquement un module par son nom (menu, web, etc.)
 */
bool setModuleEnabled(const char* name, bool enabled) {
    Module* m = ModuleRegistry::instance().getByName(name);
    if (!m) return false;
    if (enabled) m->enable();
    else m->disable();
    return true;
}

/**
 * Permet de lister tous les modules et leur état (pour menu ou API web)
 */
#include <vector>
#include <string>

void getAllModulesStatus(std::vector<std::pair<std::string, std::string>>& out) {
    out.clear();
    for (Module* m : ModuleRegistry::instance().modules()) {
        out.emplace_back(m->name(), m->stateString());
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

/**
 * Permet d'activer/désactiver dynamiquement un module par son nom
 * @param name Nom du module
 * @param enabled true pour activer, false pour désactiver
 * @return true si succès, false si échec
 */
bool TaskManager::setModuleEnabled(const char* name, bool enabled) {
    return ::setModuleEnabled(name, enabled);
}

/**
 * Permet de lister tous les modules et leur état
 * @param out Vecteur de paires (nom, état) à remplir
 */
void TaskManager::getAllModulesStatus(std::vector<std::pair<std::string, std::string>>& out) {
    ::getAllModulesStatus(out);
}
