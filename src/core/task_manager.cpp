/*
  -----------------------
  Kite PiloteV3 - Template de Fichier
  -----------------------

  Objectif : Décrire les objectifs et les choix d'architecture pour ce fichier.
  
  Instructions :
  - Ajouter des commentaires pour expliquer les sections importantes.
  - Respecter les conventions de codage définies dans le projet.
  - Documenter les fonctions et les classes pour faciliter la maintenance.

  Date : 6 mai 2025
  Auteur : Équipe Kite PiloteV3
*/

/*
  -----------------------
  Kite PiloteV3 - Module de gestion des tâches (Implémentation)
  -----------------------
  
  Implémentation optimisée des fonctions du module de gestion des tâches multiples et parallèles.
  
  Version: 2.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

// Configurations nécessaires pour vTaskList
#define configUSE_TRACE_FACILITY 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1

#include "core/task_manager.h"
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

// Initialisation des variables statiques
QueueHandle_t TaskManager::messageQueue = nullptr;
SemaphoreHandle_t TaskManager::displayMutex = nullptr;
UIManager* TaskManager::uiManager = nullptr;
WiFiManager* TaskManager::wifiManager = nullptr;

TaskManager::TaskManager() : 
    displayTaskHandle(nullptr),
    buttonTaskHandle(nullptr),
    wifiMonitorTaskHandle(nullptr),
    systemMonitorTaskHandle(nullptr),
    running(false),
    tasksRunning(false),
    lastTaskMetricsTime(0) {
    for (int i = 0; i < MAX_TASKS; i++) {
        taskHandles[i] = nullptr;
        taskParams[i] = nullptr;
        taskStats[i] = {0, 0, 0};
    }
}

TaskManager::~TaskManager() {
    stopAllTasks();
}

/**
 * Initialise le gestionnaire de tâches
 * @param ui Pointeur vers l'instance UIManager
 * @param wifi Pointeur vers l'instance WiFiManager
 * @return true si l'initialisation a réussi, false sinon
 */
bool TaskManager::begin(UIManager* ui, WiFiManager* wifi) {
    LOG_INFO("TASK", "Initialisation du gestionnaire de tâches");
    
    // Stocker les références aux dépendances
    uiManager = ui;
    wifiManager = wifi;
    
    // Initialiser les timestamps
    lastTaskMetricsTime = 0;
    tasksRunning = false;
    running = false;
    
    // Initialiser les structures de tâches avec des valeurs par défaut
    for (int i = 0; i < MAX_TASKS; i++) {
        taskHandles[i] = NULL;
        taskParams[i] = NULL;
        taskStats[i] = {0, 0, 0};
    }
    
    LOG_INFO("TASK", "Gestionnaire de tâches initialisé avec succès");
    return true;
}

/**
 * Démarre les tâches définies
 * @return true si toutes les tâches sont démarrées, false sinon
 */
bool TaskManager::startTasks() {
    LOG_INFO("TASK", "Démarrage des tâches...");
    
    if (tasksRunning) {
        LOG_WARNING("TASK", "Les tâches sont déjà en cours d'exécution");
        return true;
    }
    
    // Définition des priorités et tailles de pile adaptées à chaque tâche
    const TaskConfig taskConfigs[] = {
        // Nom, Fonction, Taille de pile, Priorité, Coeur, Période (ms), Temps réel
        {"Display", displayTask, 2048, 2, 0, 100, false},
        {"Control", controlTask, 3072, 3, 1, 20, true},
        {"Input", inputTask, 2048, 2, 0, 50, false},
        {"WiFi", networkTask, 4096, 1, 0, 100, false},
        {"Monitor", monitorTask, 2048, 1, 1, 2000, false},
        {"Sensors", sensorTask, 2048, 1, 1, 100, false}
    };
    
    const int numTasks = sizeof(taskConfigs) / sizeof(TaskConfig);
    
    // Démarrer chaque tâche avec les paramètres optimisés
    for (int i = 0; i < numTasks && i < MAX_TASKS; i++) {
        const TaskConfig& config = taskConfigs[i];
        
        // Créer les paramètres pour la tâche
        TaskParams* params = new TaskParams();
        if (!params) {
            LOG_ERROR("TASK", "Erreur d'allocation mémoire pour les paramètres de la tâche %s", config.name);
            stopAllTasks();
            return false;
        }
        
        // Configurer les paramètres
        params->taskIndex = i;
        params->manager = this;
        params->period = config.period;
        params->isRealtime = config.isRealtime;
        
        // Stocker les paramètres pour la libération future
        taskParams[i] = params;
        
        LOG_INFO("TASK", "Création de la tâche %s (pile: %d, priorité: %d, coeur: %d)",
                config.name, config.stackSize, config.priority, config.core);
        
        // Créer la tâche sur le coeur spécifié
        BaseType_t result = xTaskCreatePinnedToCore(
            config.function,        // Fonction de tâche
            config.name,            // Nom de la tâche
            config.stackSize,       // Taille de la pile
            params,                 // Paramètres
            config.priority,        // Priorité
            &taskHandles[i],        // Handle de tâche
            config.core             // Coeur (0 ou 1)
        );
        
        if (result != pdPASS) {
            LOG_ERROR("TASK", "Échec de création de la tâche %s (erreur: %d)", config.name, result);
            stopAllTasks();
            return false;
        }
    }
    
    // Marquer les tâches comme en cours d'exécution
    tasksRunning = true;
    running = true;
    LOG_INFO("TASK", "Toutes les tâches ont été démarrées avec succès");
    
    return true;
}

/**
 * Arrête proprement toutes les tâches actives
 */
void TaskManager::stopAllTasks() {
    LOG_INFO("TASK", "Arrêt de toutes les tâches");
    
    if (!tasksRunning) {
        LOG_INFO("TASK", "Aucune tâche en cours d'exécution");
        return;
    }
    
    // Supprimer chaque tâche active
    for (int i = 0; i < MAX_TASKS; i++) {
        if (taskHandles[i] != NULL) {
            vTaskDelete(taskHandles[i]);
            taskHandles[i] = NULL;
            
            // Libérer les paramètres associés
            if (taskParams[i] != NULL) {
                delete taskParams[i];
                taskParams[i] = NULL;
            }
        }
    }
    
    tasksRunning = false;
    running = false;
    LOG_INFO("TASK", "Toutes les tâches ont été arrêtées");
}

/**
 * Alias pour stopAllTasks pour compatibilité avec l'API existante
 */
void TaskManager::stopTasks() {
    stopAllTasks();
}

/**
 * Tâche de surveillance du système
 * Vérifie l'état des autres tâches et ressources système
 */
void TaskManager::monitorTask(void* parameters) {
    TaskParams* params = static_cast<TaskParams*>(parameters);
    TaskManager* manager = static_cast<TaskManager*>(params->manager);
    const TickType_t xDelay = pdMS_TO_TICKS(params->period);
    
    LOG_INFO("MONITOR", "Tâche de surveillance démarrée");
    
    while (true) {
        // Récupérer et afficher les métriques des tâches
        manager->updateTaskMetrics();
        
        // Vérifier si des tâches critiques sont bloquées
        bool allTasksHealthy = manager->checkTasksHealth();
        
        if (!allTasksHealthy) {
            LOG_WARNING("MONITOR", "Problème détecté dans une ou plusieurs tâches");
            // On pourrait implémenter une logique de récupération ici
        }
        
        // Vérifier l'utilisation de la mémoire
        uint32_t freeHeap = ESP.getFreeHeap();
        uint32_t minFreeHeap = ESP.getMinFreeHeap();
        
        if (freeHeap < 10000) {
            LOG_WARNING("MONITOR", "Mémoire faible: %u octets libres", freeHeap);
        }
        
        // Période de la tâche (non-bloquante, compatible watchdog)
        vTaskDelay(xDelay);
    }
}

/**
 * Collecte les métriques des tâches en cours d'exécution
 */
void TaskManager::updateTaskMetrics() {
    unsigned long now = millis();
    
    // Limiter la fréquence de collecte des métriques
    if (now - lastTaskMetricsTime < 5000) {
        return;
    }
    
    lastTaskMetricsTime = now;
    
    // Collecter les métriques pour chaque tâche
    for (int i = 0; i < MAX_TASKS; i++) {
        if (taskHandles[i] != NULL) {
            // Mesurer l'utilisation de la pile
            taskStats[i].stackHighWaterMark = uxTaskGetStackHighWaterMark(taskHandles[i]);
            
            // Si la pile restante est trop faible, émettre un avertissement
            if (taskStats[i].stackHighWaterMark < 128) {
                LOG_WARNING("TASK", "Tâche %d: pile presque pleine (marge: %u octets)",
                           i, taskStats[i].stackHighWaterMark * sizeof(StackType_t));
            }
            
            // On pourrait ajouter d'autres métriques ici (temps CPU, etc.)
        }
    }
    
    // Afficher un résumé des ressources système
    LOG_INFO("SYS", "Heap: %u/%u (%d%%), Min: %u",
             ESP.getFreeHeap(),
             ESP.getHeapSize(),
             ESP.getFreeHeap() * 100 / ESP.getHeapSize(),
             ESP.getMinFreeHeap());
}

/**
 * Vérifie la santé des tâches en cours d'exécution
 * @return true si toutes les tâches sont en bonne santé, false sinon
 */
bool TaskManager::checkTasksHealth() {
    bool allHealthy = true;
    
    // Vérifier l'état de chaque tâche
    for (int i = 0; i < MAX_TASKS; i++) {
        if (taskHandles[i] != NULL) {
            // Vérifier si la tâche est bloquée ou a débordé sa pile
            eTaskState state = eTaskGetState(taskHandles[i]);
            
            if (state == eBlocked && taskParams[i] && taskParams[i]->isRealtime) {
                LOG_WARNING("TASK", "Tâche %d bloquée (temps réel)", i);
                allHealthy = false;
            }
            
            if (taskStats[i].stackHighWaterMark < 64) {
                LOG_WARNING("TASK", "Tâche %d: débordement de pile imminent", i);
                allHealthy = false;
            }
        }
    }
    
    return allHealthy;
}

// Implémentations des fonctions de tâches

void TaskManager::displayTask(void* parameters) {
    TaskParams* params = static_cast<TaskParams*>(parameters);
    const TickType_t xDelay = pdMS_TO_TICKS(params->period);
    LOG_INFO("DISPLAY_TASK", "Tâche d'affichage démarrée");
    while (true) {
        // Mettre à jour l'affichage principal (LCD)
        extern DisplayManager display;
        extern PotentiometerManager potManager;
        int dir = potManager.getDirection();
        int trim = potManager.getTrim();
        int len = potManager.getLineLength();
        bool wifi = WiFi.isConnected();
        unsigned long uptime = millis() / 1000;
        display.displayLiveStatus(dir, trim, len, wifi, uptime);
        vTaskDelay(xDelay);
    }
}

void TaskManager::inputTask(void* parameters) {
    TaskParams* params = static_cast<TaskParams*>(parameters);
    const TickType_t xDelay = pdMS_TO_TICKS(params->period);
    LOG_INFO("INPUT_TASK", "Tâche de lecture des potentiomètres démarrée");
    while (true) {
        extern PotentiometerManager potManager;
        potManager.updatePotentiometers();
        vTaskDelay(xDelay);
    }
}

void TaskManager::controlTask(void* parameters) {
    TaskParams* params = static_cast<TaskParams*>(parameters);
    const TickType_t xDelay = pdMS_TO_TICKS(params->period);
    LOG_INFO("CONTROL_TASK", "Tâche de contrôle moteur démarrée");
    while (true) {
        extern PotentiometerManager potManager;
        int dir = potManager.getDirection();
        int trim = potManager.getTrim();
        int len = potManager.getLineLength();
        servoUpdateAll(dir, trim, len);
        vTaskDelay(xDelay);
    }
}

void TaskManager::buttonTask(void* parameters) {
    TaskParams* params = static_cast<TaskParams*>(parameters);
    const TickType_t xDelay = pdMS_TO_TICKS(params->period);
    LOG_INFO("BUTTON_TASK", "Tâche de gestion des boutons démarrée");
    while (true) {
        extern ButtonUIManager buttonUI;
        buttonUI.update();
        vTaskDelay(xDelay);
    }
}

void TaskManager::networkTask(void* parameters) {
    TaskParams* params = static_cast<TaskParams*>(parameters);
    const TickType_t xDelay = pdMS_TO_TICKS(params->period);
    LOG_INFO("NETWORK_TASK", "Tâche réseau démarrée");
    while (true) {
        ElegantOTA.loop();
        vTaskDelay(xDelay);
    }
}

void TaskManager::sensorTask(void* parameters) {
    TaskParams* params = static_cast<TaskParams*>(parameters);
    const TickType_t xDelay = pdMS_TO_TICKS(params->period);
    LOG_INFO("SENSOR_TASK", "Tâche de lecture des capteurs démarrée");
    
    // Variables pour la surveillance de l'état des capteurs
    uint32_t sensorFailures[4] = {0}; // Compteurs d'échecs pour chaque capteur
    const uint32_t MAX_FAILURES = 5;  // Nombre d'échecs consécutifs avant signalement
    bool sensorStatus[4] = {false};   // État actuel de chaque capteur
    
    // Initialisation des capteurs
    #ifdef USE_IMU
    bool imuInitialized = imuInit();
    LOG_INFO("SENSOR_TASK", "Initialisation IMU: %s", imuInitialized ? "OK" : "ÉCHEC");
    sensorStatus[0] = imuInitialized;
    #endif
    
    #ifdef USE_LINE_LENGTH_SENSOR
    bool lineLengthInitialized = lineLengthInit();
    LOG_INFO("SENSOR_TASK", "Initialisation capteur longueur: %s", lineLengthInitialized ? "OK" : "ÉCHEC");
    sensorStatus[1] = lineLengthInitialized;
    #endif
    
    #ifdef USE_TENSION_SENSOR
    bool tensionInitialized = tensionInit();
    LOG_INFO("SENSOR_TASK", "Initialisation capteur tension: %s", tensionInitialized ? "OK" : "ÉCHEC");
    sensorStatus[2] = tensionInitialized;
    #endif
    
    #ifdef USE_WIND_SENSOR
    bool windInitialized = windInit();
    LOG_INFO("SENSOR_TASK", "Initialisation capteur vent: %s", windInitialized ? "OK" : "ÉCHEC");
    sensorStatus[3] = windInitialized;
    #endif
    
    // Boucle principale de la tâche
    while (true) {
        // Mise à jour des capteurs avec gestion d'erreurs
        
        #ifdef USE_IMU
        bool imuResult = updateIMU();
        // Gestion des échecs consécutifs
        if (!imuResult) {
            sensorFailures[0]++;
            if (sensorFailures[0] >= MAX_FAILURES && sensorStatus[0]) {
                LOG_ERROR("SENSOR_TASK", "IMU: échec après %d tentatives", sensorFailures[0]);
                sensorStatus[0] = false;
            }
        } else {
            // Réinitialiser le compteur en cas de succès
            if (sensorFailures[0] > 0 && !sensorStatus[0]) {
                LOG_INFO("SENSOR_TASK", "IMU: récupération après %d échecs", sensorFailures[0]);
                sensorStatus[0] = true;
            }
            sensorFailures[0] = 0;
        }
        #endif
        
        #ifdef USE_LINE_LENGTH_SENSOR
        bool lineLengthResult = updateLineLengthSensor();
        if (!lineLengthResult) {
            sensorFailures[1]++;
            if (sensorFailures[1] >= MAX_FAILURES && sensorStatus[1]) {
                LOG_ERROR("SENSOR_TASK", "Capteur longueur: échec après %d tentatives", sensorFailures[1]);
                sensorStatus[1] = false;
            }
        } else {
            if (sensorFailures[1] > 0 && !sensorStatus[1]) {
                LOG_INFO("SENSOR_TASK", "Capteur longueur: récupération après %d échecs", sensorFailures[1]);
                sensorStatus[1] = true;
            }
            sensorFailures[1] = 0;
        }
        #endif
        
        #ifdef USE_TENSION_SENSOR
        bool tensionResult = updateTensionSensor();
        if (!tensionResult) {
            sensorFailures[2]++;
            if (sensorFailures[2] >= MAX_FAILURES && sensorStatus[2]) {
                LOG_ERROR("SENSOR_TASK", "Capteur tension: échec après %d tentatives", sensorFailures[2]);
                sensorStatus[2] = false;
            }
        } else {
            if (sensorFailures[2] > 0 && !sensorStatus[2]) {
                LOG_INFO("SENSOR_TASK", "Capteur tension: récupération après %d échecs", sensorFailures[2]);
                sensorStatus[2] = true;
            }
            sensorFailures[2] = 0;
        }
        #endif
        
        #ifdef USE_WIND_SENSOR
        bool windResult = updateWindSensor();
        if (!windResult) {
            sensorFailures[3]++;
            if (sensorFailures[3] >= MAX_FAILURES && sensorStatus[3]) {
                LOG_ERROR("SENSOR_TASK", "Capteur vent: échec après %d tentatives", sensorFailures[3]);
                sensorStatus[3] = false;
            }
        } else {
            if (sensorFailures[3] > 0 && !sensorStatus[3]) {
                LOG_INFO("SENSOR_TASK", "Capteur vent: récupération après %d échecs", sensorFailures[3]);
                sensorStatus[3] = true;
            }
            sensorFailures[3] = 0;
        }
        #endif
        
        // Tentative de réinitialisation périodique des capteurs défaillants
        // toutes les 30 secondes (300 itérations avec un délai de 100ms)
        static uint32_t resetCounter = 0;
        resetCounter++;
        
        if (resetCounter >= 300) {
            resetCounter = 0;
            
            // Tenter de réinitialiser les capteurs défaillants
            #ifdef USE_IMU
            if (!sensorStatus[0]) {
                LOG_INFO("SENSOR_TASK", "Tentative de réinitialisation de l'IMU");
                bool result = imuInit();
                if (result) {
                    LOG_INFO("SENSOR_TASK", "IMU réinitialisé avec succès");
                    sensorStatus[0] = true;
                    sensorFailures[0] = 0;
                }
            }
            #endif
            
            #ifdef USE_LINE_LENGTH_SENSOR
            if (!sensorStatus[1]) {
                LOG_INFO("SENSOR_TASK", "Tentative de réinitialisation du capteur de longueur");
                bool result = lineLengthInit();
                if (result) {
                    LOG_INFO("SENSOR_TASK", "Capteur de longueur réinitialisé avec succès");
                    sensorStatus[1] = true;
                    sensorFailures[1] = 0;
                }
            }
            #endif
            
            #ifdef USE_TENSION_SENSOR
            if (!sensorStatus[2]) {
                LOG_INFO("SENSOR_TASK", "Tentative de réinitialisation du capteur de tension");
                bool result = tensionInit();
                if (result) {
                    LOG_INFO("SENSOR_TASK", "Capteur de tension réinitialisé avec succès");
                    sensorStatus[2] = true;
                    sensorFailures[2] = 0;
                }
            }
            #endif
            
            #ifdef USE_WIND_SENSOR
            if (!sensorStatus[3]) {
                LOG_INFO("SENSOR_TASK", "Tentative de réinitialisation du capteur de vent");
                bool result = windInit();
                if (result) {
                    LOG_INFO("SENSOR_TASK", "Capteur de vent réinitialisé avec succès");
                    sensorStatus[3] = true;
                    sensorFailures[3] = 0;
                }
            }
            #endif
        }
        
        // Attente non-bloquante compatible avec FreeRTOS
        vTaskDelay(xDelay);
    }
}
