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

#include "../core/task_manager.h"
#include "../include/logging.h"

// Initialisation des variables statiques
QueueHandle_t TaskManager::messageQueue = nullptr;
SemaphoreHandle_t TaskManager::displayMutex = nullptr;
DisplayManager* TaskManager::displayManager = nullptr;
AsyncWebServer* TaskManager::webServer = nullptr;

// Structure pour les informations de performance des tâches
typedef struct {
  unsigned long lastExecutionTime;
  unsigned long maxExecutionTime;
  unsigned long totalExecutionTime;
  unsigned long executionCount;
} TaskMetrics;

// Métriques de performance pour les tâches 
static TaskMetrics displayMetrics = {0, 0, 0, 0};
static TaskMetrics touchMetrics = {0, 0, 0, 0};
static TaskMetrics wifiMetrics = {0, 0, 0, 0};
static TaskMetrics systemMetrics = {0, 0, 0, 0};

/**
 * Constructeur par défaut
 */
TaskManager::TaskManager() : 
  displayTaskHandle(nullptr),
  touchTaskHandle(nullptr),
  wifiMonitorTaskHandle(nullptr),
  systemMonitorTaskHandle(nullptr),
  running(false) {
}

/**
 * Destructeur
 */
TaskManager::~TaskManager() {
  stopTasks();
}

/**
 * Initialise le gestionnaire de tâches avec les références aux objets nécessaires
 * @param display Référence au gestionnaire d'affichage
 * @param server Référence au serveur web
 */
void TaskManager::begin(DisplayManager* display, AsyncWebServer* server) {
  LOG_INFO("TASK", "Initialisation du gestionnaire de tâches");
  
  // Stocker les références aux objets
  displayManager = display;
  webServer = server;
  
  // Créer la file d'attente pour la communication inter-tâches
  messageQueue = xQueueCreate(10, sizeof(TaskMessage));
  if (messageQueue == nullptr) {
    LOG_ERROR("TASK", "Impossible de créer la file d'attente de messages");
    return;
  }
  
  // Créer le mutex pour l'accès à l'affichage
  displayMutex = xSemaphoreCreateMutex();
  if (displayMutex == nullptr) {
    LOG_ERROR("TASK", "Impossible de créer le mutex d'affichage");
    return;
  }
  
  LOG_INFO("TASK", "Gestionnaire de tâches initialisé avec succès");
  logMemoryUsage("TASK");
}

/**
 * Démarre toutes les tâches
 */
void TaskManager::startTasks() {
  if (running) {
    LOG_WARNING("TASK", "Les tâches sont déjà en cours d'exécution");
    return;
  }
  
  LOG_INFO("TASK", "Démarrage des tâches...");
  
  // Créer et démarrer la tâche d'affichage
  xTaskCreatePinnedToCore(
    displayTask,                   // Fonction de tâche
    "DisplayTask",                 // Nom de la tâche
    STACK_SIZE_DISPLAY,            // Taille de la pile
    nullptr,                       // Paramètres
    TASK_PRIORITY_DISPLAY,         // Priorité
    &displayTaskHandle,            // Handle
    1                              // Core (1 = Application Core)
  );
  
  // Créer et démarrer la tâche de surveillance WiFi
  xTaskCreatePinnedToCore(
    wifiMonitorTask,               // Fonction de tâche
    "WiFiMonitorTask",             // Nom de la tâche
    STACK_SIZE_WIFI_MONITOR,       // Taille de la pile
    nullptr,                       // Paramètres
    TASK_PRIORITY_WIFI_MONITOR,    // Priorité
    &wifiMonitorTaskHandle,        // Handle
    0                              // Core (0 = Protocol Core)
  );
  
  // Créer et démarrer la tâche de surveillance système
  xTaskCreatePinnedToCore(
    systemMonitorTask,             // Fonction de tâche
    "SystemMonitorTask",           // Nom de la tâche
    STACK_SIZE_SYSTEM_MONITOR,     // Taille de la pile
    nullptr,                       // Paramètres
    TASK_PRIORITY_SYSTEM_MONITOR,  // Priorité
    &systemMonitorTaskHandle,      // Handle
    0                              // Core (0 = Protocol Core)
  );
  
  running = true;
  LOG_INFO("TASK", "Toutes les tâches ont été démarrées avec succès");
  
  // Enregistrer l'utilisation de la mémoire après le démarrage des tâches
  logMemoryUsage("TASK");
}

/**
 * Arrête toutes les tâches
 */
void TaskManager::stopTasks() {
  if (!running) {
    return;
  }
  
  Serial.println("Arrêt des tâches...");
  
  // Supprimer les tâches
  if (displayTaskHandle != nullptr) {
    vTaskDelete(displayTaskHandle);
    displayTaskHandle = nullptr;
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
  Serial.println("Toutes les tâches ont été arrêtées");
}

/**
 * Envoie un message dans la file d'attente pour communication inter-tâches
 * @param type Type de message
 * @param value Valeur associée
 * @param message Message textuel (optionnel)
 * @return true si le message a été envoyé, false sinon
 */
bool TaskManager::sendMessage(MessageType type, uint16_t value, const char* message) {
  if (messageQueue == nullptr) {
    return false;
  }
  
  TaskMessage msg;
  msg.type = type;
  msg.value = value;
  msg.message = String(message);
  
  if (xQueueSend(messageQueue, &msg, pdMS_TO_TICKS(50)) != pdPASS) {
    Serial.println("ERREUR: Impossible d'envoyer le message (file pleine)");
    return false;
  }
  
  return true;
}

/**
 * Vérifie si les tâches sont en cours d'exécution
 * @return true si les tâches sont en cours d'exécution, false sinon
 */
bool TaskManager::isRunning() const {
  return running;
}

/**
 * Tâche pour la gestion de l'affichage - exécution sur le core 1 (version optimisée)
 * @param parameter Paramètres de la tâche (non utilisés)
 */
void TaskManager::displayTask(void* parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  
  // Cache pour l'état réseau précédent pour éviter les mises à jour inutiles
  static bool wasConnected = false;
  static String lastSSID = "";
  static IPAddress lastIP;
  
  // Remplacer vTaskDelay par vTaskDelayUntil pour une exécution à intervalle précis
  while (true) {
    unsigned long startTime = millis();
    
    // Prendre le sémaphore pour accéder à l'affichage - timeout réduit
    if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      // Traiter les messages de mise à jour d'affichage en priorité
      TaskMessage msg;
      while (xQueueReceive(messageQueue, &msg, 0) == pdPASS) {
        if (msg.type == MSG_DISPLAY_UPDATE) {
          processMessage(msg);
        }
      }
      
      // Mettre à jour l'affichage seulement si nécessaire
      if (displayManager != nullptr && displayManager->isInitialized()) {
        // Vérifier s'il y a des changements dans la connexion WiFi
        bool connected = (WiFi.status() == WL_CONNECTED);
        String currentSSID = WiFi.SSID();
        IPAddress currentIP = WiFi.localIP();
        
        // Mettre à jour l'affichage uniquement si l'état a changé
        if (connected != wasConnected || currentSSID != lastSSID || currentIP != lastIP) {
          wasConnected = connected;
          lastSSID = currentSSID;
          lastIP = currentIP;
          
          // Rotation d'écran optimisée pour éviter les rafraîchissements inutiles
          if (connected) {
            displayManager->updateDisplayRotation(currentSSID, currentIP);
          } else {
            displayManager->displayMessage("WiFi", "Déconnecté", true);
          }
        }
      }
      
      // Libérer le sémaphore immédiatement après utilisation
      xSemaphoreGive(displayMutex);
    }
    
    // Mettre à jour les métriques de performance
    unsigned long executionTime = millis() - startTime;
    displayMetrics.lastExecutionTime = executionTime;
    displayMetrics.totalExecutionTime += executionTime;
    displayMetrics.executionCount++;
    if (executionTime > displayMetrics.maxExecutionTime) {
      displayMetrics.maxExecutionTime = executionTime;
    }
    
    // Attendre jusqu'au prochain intervalle, avec sécurité anti-overflow
    if (pdMS_TO_TICKS(DISPLAY_UPDATE_INTERVAL) > executionTime) {
      vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(DISPLAY_UPDATE_INTERVAL));
    } else {
      // Réinitialiser lastWakeTime si l'exécution a pris trop de temps
      lastWakeTime = xTaskGetTickCount();
    }
  }
}

/**
 * Tâche pour la surveillance du WiFi - exécution sur le core 0 (version optimisée)
 * @param parameter Paramètres de la tâche (non utilisés)
 */
void TaskManager::wifiMonitorTask(void* parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  static uint8_t disconnectedCount = 0;
  
  while (true) {
    unsigned long startTime = millis();
    
    // Vérifier l'état de la connexion WiFi
    bool connected = (WiFi.status() == WL_CONNECTED);
    
    // Envoyer un message uniquement si l'état a changé
    static bool lastConnectedState = false;
    if (connected != lastConnectedState) {
      lastConnectedState = connected;
      sendMessage(MSG_WIFI_STATUS, (uint16_t)(connected ? 1 : 0), connected ? "Connecté" : "Déconnecté");
      
      if (connected) {
        // Réinitialiser le compteur de déconnexions
        disconnectedCount = 0;
        
        // Forcer le rafraîchissement de l'affichage
        if (displayManager != nullptr) {
          displayManager->forceRefresh();
        }
      }
    }
    
    // Si déconnecté, utiliser une stratégie de reconnexion progressive
    if (!connected) {
      disconnectedCount++;
      
      // Ne tenter la reconnexion qu'après plusieurs déconnexions consécutives
      // pour éviter une utilisation excessive des ressources
      if (disconnectedCount >= 3) {
        LOG_WARNING("WIFI", "WiFi déconnecté, tentative de reconnexion...");
        WiFi.reconnect();
        disconnectedCount = 0;
        
        // Ajouter un délai supplémentaire après une tentative de reconnexion
        vTaskDelay(pdMS_TO_TICKS(500));
      }
    }
    
    // Mettre à jour les métriques de performance
    unsigned long executionTime = millis() - startTime;
    wifiMetrics.lastExecutionTime = executionTime;
    wifiMetrics.totalExecutionTime += executionTime;
    wifiMetrics.executionCount++;
    if (executionTime > wifiMetrics.maxExecutionTime) {
      wifiMetrics.maxExecutionTime = executionTime;
    }
    
    // Utiliser un intervalle variable: vérifier plus souvent si déconnecté
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(connected ? WIFI_CHECK_INTERVAL : WIFI_CHECK_INTERVAL/2));
  }
}

/**
 * Tâche pour la surveillance du système - exécution sur le core 0 (version optimisée)
 * @param parameter Paramètres de la tâche (non utilisés)
 */
void TaskManager::systemMonitorTask(void* parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  
  while (true) {
    unsigned long startTime = millis();
    
    // Collecter les informations système
    uint32_t freeHeap = ESP.getFreeHeap();
    uint8_t cpuTemp = temperatureRead(); // Lecture de la température du CPU
    
    // Envoyer un message avec les statistiques système seulement en cas de changement significatif
    static uint32_t lastFreeHeap = 0;
    static uint8_t lastCpuTemp = 0;
    
    if (abs((int32_t)freeHeap - (int32_t)lastFreeHeap) > 1024 || abs((int16_t)cpuTemp - (int16_t)lastCpuTemp) > 2) {
      lastFreeHeap = freeHeap;
      lastCpuTemp = cpuTemp;
      
      char statsBuffer[48];
      snprintf(statsBuffer, sizeof(statsBuffer), "Heap: %u KB, Temp: %u°C", freeHeap / 1024, cpuTemp);
      sendMessage(MSG_SYSTEM_STATUS, (uint16_t)(freeHeap / 1024), statsBuffer);
    }
    
    // Vérifier la charge CPU et les métriques des tâches périodiquement
    static uint8_t metricCounter = 0;
    if (++metricCounter >= 5) {
      metricCounter = 0;
      
      LOG_INFO("PERF", "Display: Avg=%lu ms, Max=%lu ms", 
               displayMetrics.executionCount > 0 ? displayMetrics.totalExecutionTime / displayMetrics.executionCount : 0,
               displayMetrics.maxExecutionTime);
      
      LOG_INFO("PERF", "Touch: Avg=%lu ms, Max=%lu ms", 
               touchMetrics.executionCount > 0 ? touchMetrics.totalExecutionTime / touchMetrics.executionCount : 0,
               touchMetrics.maxExecutionTime);
      
      // Réinitialiser certaines métriques pour la prochaine période
      if (displayMetrics.executionCount > 1000) {
        displayMetrics.totalExecutionTime = 0;
        displayMetrics.executionCount = 0;
      }
      
      if (touchMetrics.executionCount > 1000) {
        touchMetrics.totalExecutionTime = 0;
        touchMetrics.executionCount = 0;
      }
    }
    
    // Afficher périodiquement le graphique d'utilisation mémoire, mais moins fréquemment
    static uint8_t graphCounter = 0;
    if (++graphCounter >= 10) { // Augmenté de 5 à 10
      graphCounter = 0;
      logMemoryGraph();
    } else if (graphCounter == 5) {
      // À mi-chemin, enregistrer juste l'utilisation sans le graphique
      logMemoryUsage("SYSTEM");
    }
    
    // Mettre à jour les métriques de performance
    unsigned long executionTime = millis() - startTime;
    systemMetrics.lastExecutionTime = executionTime;
    systemMetrics.totalExecutionTime += executionTime;
    systemMetrics.executionCount++;
    if (executionTime > systemMetrics.maxExecutionTime) {
      systemMetrics.maxExecutionTime = executionTime;
    }
    
    // Attendre jusqu'au prochain intervalle
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(SYSTEM_CHECK_INTERVAL));
  }
}

/**
 * Traite un message reçu dans la file d'attente
 * @param msg Message à traiter
 */
void TaskManager::processMessage(const TaskMessage& msg) {
  switch (msg.type) {
    case MSG_DISPLAY_UPDATE:
      // Traiter les messages de mise à jour d'affichage
      LOG_INFO("MSG", "Affichage: %s", msg.message.c_str());
      break;
      
    case MSG_TOUCH_EVENT:
      // Traiter les événements tactiles
      LOG_INFO("MSG", "Tactile: %d", msg.value);
      break;
      
    case MSG_WIFI_STATUS:
      // Traiter les mises à jour d'état WiFi
      LOG_INFO("MSG", "WiFi: %s", msg.value ? "Connecté" : "Déconnecté");
      break;
      
    case MSG_SYSTEM_STATUS:
      // Traiter les mises à jour d'état système
      LOG_INFO("MSG", "Système: %s", msg.message.c_str());
      break;
      
    case MSG_ERROR:
      // Traiter les messages d'erreur
      LOG_ERROR("MSG", "%s", msg.message.c_str());
      break;
      
    case MSG_TASK_LIST:
      // Traiter les listes de tâches
      LOG_INFO("MSG", "Liste des tâches:\n%s", msg.message.c_str());
      break;
      
    default:
      // Message inconnu
      Serial.printf("Message inconnu reçu: type=%d, valeur=%d\n", msg.type, msg.value);
      break;
  }
}
