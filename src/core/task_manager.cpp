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
    running(false) {
}

TaskManager::~TaskManager() {
    stopTasks();
}

void TaskManager::begin(UIManager* ui, WiFiManager* wifi) {
    LOG_INFO("TASK", "Initialisation du gestionnaire de tâches");
    
    uiManager = ui;
    wifiManager = wifi;
    
    messageQueue = xQueueCreate(10, sizeof(TaskMessage));
    if (messageQueue == nullptr) {
        LOG_ERROR("TASK", "Impossible de créer la file d'attente de messages");
        return;
    }
    
    displayMutex = xSemaphoreCreateMutex();
    if (displayMutex == nullptr) {
        LOG_ERROR("TASK", "Impossible de créer le mutex d'affichage");
        return;
    }
    
    LOG_INFO("TASK", "Gestionnaire de tâches initialisé avec succès");
}

void TaskManager::startTasks() {
    if (running) {
        LOG_WARNING("TASK", "Les tâches sont déjà en cours d'exécution");
        return;
    }
    
    LOG_INFO("TASK", "Démarrage des tâches...");
    
    // Tâche d'affichage
    xTaskCreatePinnedToCore(
        displayTask,
        "DisplayTask",
        DISPLAY_TASK_STACK_SIZE,
        nullptr,
        DISPLAY_TASK_PRIORITY,
        &displayTaskHandle,
        1  // Core 1
    );
    
    // Tâche de gestion des boutons
    xTaskCreatePinnedToCore(
        buttonTask,
        "ButtonTask",
        DISPLAY_TASK_STACK_SIZE,
        nullptr,
        DISPLAY_TASK_PRIORITY,
        &buttonTaskHandle,
        1  // Core 1
    );
    
    // Tâche de surveillance WiFi
    xTaskCreatePinnedToCore(
        wifiMonitorTask,
        "WiFiMonitorTask",
        WIFI_TASK_STACK_SIZE,
        nullptr,
        WIFI_TASK_PRIORITY,
        &wifiMonitorTaskHandle,
        0  // Core 0
    );
    
    // Tâche de surveillance système
    xTaskCreatePinnedToCore(
        systemMonitorTask,
        "SystemMonitorTask",
        SYSTEM_TASK_STACK_SIZE,
        nullptr,
        WIFI_TASK_PRIORITY,
        &systemMonitorTaskHandle,
        0  // Core 0
    );
    
    running = true;
    LOG_INFO("TASK", "Toutes les tâches ont été démarrées avec succès");
}

void TaskManager::stopTasks() {
    if (!running) return;
    
    if (displayTaskHandle != nullptr) {
        vTaskDelete(displayTaskHandle);
        displayTaskHandle = nullptr;
    }
    
    if (buttonTaskHandle != nullptr) {
        vTaskDelete(buttonTaskHandle);
        buttonTaskHandle = nullptr;
    }
    
    if (wifiMonitorTaskHandle != nullptr) {
        vTaskDelete(wifiMonitorTaskHandle);
        wifiMonitorTaskHandle = nullptr;
    }
    
    if (systemMonitorTaskHandle != nullptr) {
        vTaskDelete(systemMonitorTaskHandle);
        systemMonitorTaskHandle = nullptr;
    }
    
    running = false;
    LOG_INFO("TASK", "Toutes les tâches ont été arrêtées");
}

bool TaskManager::sendMessage(MessageType type, uint16_t value, const char* message) {
    if (messageQueue == nullptr) return false;
    
    TaskMessage msg;
    msg.type = type;
    msg.value = value;
    msg.message = String(message);
    
    return xQueueSend(messageQueue, &msg, pdMS_TO_TICKS(50)) == pdPASS;
}

bool TaskManager::isRunning() const {
    return running;
}

void TaskManager::displayTask(void* parameter) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (true) {
        if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            if (uiManager != nullptr && uiManager->isInitialized()) {
                uiManager->updateDisplay();
            }
            xSemaphoreGive(displayMutex);
        }
        
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(DISPLAY_UPDATE_INTERVAL));
    }
}

void TaskManager::buttonTask(void* parameter) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (true) {
        if (uiManager != nullptr && uiManager->isInitialized()) {
            uiManager->checkButtons();
        }
        
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(BUTTON_CHECK_INTERVAL));
    }
}

void TaskManager::wifiMonitorTask(void* parameter) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    static uint8_t disconnectedCount = 0;
    
    while (true) {
        if (wifiManager != nullptr) {
            bool connected = wifiManager->isConnected();
            static bool lastConnectedState = false;
            
            if (connected != lastConnectedState) {
                lastConnectedState = connected;
                sendMessage(MSG_WIFI_STATUS, connected ? 1 : 0, 
                          connected ? "Connecté" : "Déconnecté");
                
                if (connected) {
                    disconnectedCount = 0;
                    if (uiManager != nullptr) {
                        uiManager->displayWiFiInfo(wifiManager->getSSID(), 
                                                 wifiManager->getLocalIP());
                    }
                }
            }
            
            if (!connected && ++disconnectedCount >= 3) {
                wifiManager->reconnect();
                disconnectedCount = 0;
                vTaskDelay(pdMS_TO_TICKS(500));
            }
        }
        
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(WIFI_CHECK_INTERVAL));
    }
}

void TaskManager::systemMonitorTask(void* parameter) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (true) {
        uint32_t freeHeap = ESP.getFreeHeap();
        uint8_t cpuTemp = temperatureRead();
        
        static uint32_t lastFreeHeap = 0;
        static uint8_t lastCpuTemp = 0;
        
        if (abs((int32_t)freeHeap - (int32_t)lastFreeHeap) > 1024 || 
            abs((int16_t)cpuTemp - (int16_t)lastCpuTemp) > 2) {
            
            lastFreeHeap = freeHeap;
            lastCpuTemp = cpuTemp;
            
            char statsBuffer[48];
            snprintf(statsBuffer, sizeof(statsBuffer), 
                    "Heap: %u KB, Temp: %u°C", freeHeap / 1024, cpuTemp);
            sendMessage(MSG_SYSTEM_STATUS, freeHeap / 1024, statsBuffer);
            
            if (uiManager != nullptr) {
                uiManager->displaySystemStats();
            }
        }
        
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(SYSTEM_CHECK_INTERVAL));
    }
}

void TaskManager::processMessage(const TaskMessage& msg) {
    switch (msg.type) {
        case MSG_DISPLAY_UPDATE:
            if (uiManager != nullptr) {
                uiManager->setDisplayNeedsUpdate(true);
            }
            break;
            
        case MSG_WIFI_STATUS:
            if (uiManager != nullptr) {
                uiManager->displayMessage("WiFi", 
                    msg.value ? "Connecté" : "Déconnecté");
            }
            break;
            
        case MSG_SYSTEM_STATUS:
            if (uiManager != nullptr) {
                uiManager->displaySystemStats();
            }
            break;
            
        case MSG_ERROR:
            LOG_ERROR("MSG", "%s", msg.message.c_str());
            if (uiManager != nullptr) {
                uiManager->displayMessage("Erreur", msg.message.c_str());
            }
            break;
            
        default:
            break;
    }
}
