/*
  -----------------------
  Kite PiloteV3 - Module de gestion des tâches (Implémentation)
  -----------------------
  
  Implémentation des fonctions du module de gestion des tâches multiples et parallèles.
  
  Version: 1.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

// Configurations nécessaires pour vTaskList
#define configUSE_TRACE_FACILITY 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1

#include "../include/task_manager.h"
#include "../include/logging.h"

// Initialisation des variables statiques
QueueHandle_t TaskManager::messageQueue = nullptr;
SemaphoreHandle_t TaskManager::displayMutex = nullptr;
DisplayManager* TaskManager::displayManager = nullptr;
TouchUIManager* TaskManager::touchUIManager = nullptr;
AsyncWebServer* TaskManager::webServer = nullptr;

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
 * @param touchUI Référence au gestionnaire d'interface tactile
 * @param server Référence au serveur web
 */
void TaskManager::begin(DisplayManager* display, TouchUIManager* touchUI, AsyncWebServer* server) {
  LOG_INFO("TASK", "Initialisation du gestionnaire de tâches");
  
  // Stocker les références aux objets
  displayManager = display;
  touchUIManager = touchUI;
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
  
  // Créer et démarrer la tâche de gestion tactile
  xTaskCreatePinnedToCore(
    touchTask,                     // Fonction de tâche
    "TouchTask",                   // Nom de la tâche
    STACK_SIZE_TOUCH,              // Taille de la pile
    nullptr,                       // Paramètres
    TASK_PRIORITY_TOUCH,           // Priorité
    &touchTaskHandle,              // Handle
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
  
  if (touchTaskHandle != nullptr) {
    vTaskDelete(touchTaskHandle);
    touchTaskHandle = nullptr;
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
 * Tâche pour la gestion de l'affichage - exécution sur le core 1
 * @param parameter Paramètres de la tâche (non utilisés)
 */
void TaskManager::displayTask(void* parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  
  // Remplacer vTaskDelay par vTaskDelayUntil pour une exécution à intervalle précis
  while (true) {
    // Prendre le sémaphore pour accéder à l'affichage
    if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      // Traiter les messages de mise à jour d'affichage
      TaskMessage msg;
      while (xQueueReceive(messageQueue, &msg, 0) == pdPASS) {
        if (msg.type == MSG_DISPLAY_UPDATE) {
          processMessage(msg);
        }
      }
      
      // Mettre à jour l'affichage normal si nécessaire
      if (displayManager != nullptr && displayManager->isInitialized()) {
        // Mise à jour de l'affichage ici si nécessaire
      }
      
      // Libérer le sémaphore
      xSemaphoreGive(displayMutex);
    }
    
    // Attendre jusqu'au prochain intervalle
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(DISPLAY_UPDATE_INTERVAL));
  }
}

/**
 * Tâche pour la gestion des événements tactiles - exécution sur le core 1
 * @param parameter Paramètres de la tâche (non utilisés)
 */
void TaskManager::touchTask(void* parameter) {
  // Commencer avec un délai pour éviter les conflits d'initialisation
  vTaskDelay(pdMS_TO_TICKS(500));
  
  TickType_t lastWakeTime = xTaskGetTickCount();
  
  while (true) {
    // Vérifier si un événement tactile est détecté
    if (touchUIManager != nullptr && displayManager != nullptr) {
      // Vérifier d'abord si l'écran tactile est initialisé
      if (displayManager->isTouchInitialized()) {
        // Prendre le sémaphore avec un timeout plus long pour éviter les timeouts
        if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
          // Traiter les événements tactiles avec protection
          try {
            touchUIManager->processTouch();
          } catch (...) {
            Serial.println("Exception dans processTouch()");
          }
          
          // Libérer le sémaphore
          xSemaphoreGive(displayMutex);
        }
      }
    }
    
    // Utiliser vTaskDelay au lieu de vTaskDelayUntil pour plus de stabilité
    vTaskDelay(pdMS_TO_TICKS(TOUCH_CHECK_INTERVAL));
  }
}

/**
 * Tâche pour la surveillance du WiFi - exécution sur le core 0
 * @param parameter Paramètres de la tâche (non utilisés)
 */
void TaskManager::wifiMonitorTask(void* parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  
  while (true) {
    // Vérifier l'état de la connexion WiFi
    bool connected = (WiFi.status() == WL_CONNECTED);
    
    // Envoyer un message avec l'état WiFi
    sendMessage(MSG_WIFI_STATUS, (uint16_t)(connected ? 1 : 0), connected ? "Connecté" : "Déconnecté");
    
    // Si déconnecté, essayer de reconnecter
    if (!connected) {
      Serial.println("WiFi déconnecté, tentative de reconnexion...");
      WiFi.reconnect();
    }
    
    // Attendre jusqu'au prochain intervalle
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(WIFI_CHECK_INTERVAL));
  }
}

/**
 * Tâche pour la surveillance du système - exécution sur le core 0
 * @param parameter Paramètres de la tâche (non utilisés)
 */
void TaskManager::systemMonitorTask(void* parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  
  while (true) {
    // Collecter les informations système
    uint32_t freeHeap = ESP.getFreeHeap();
    uint8_t cpuTemp = temperatureRead(); // Lecture de la température du CPU
    
    // Envoyer un message avec les statistiques système
    char statsBuffer[64];
    snprintf(statsBuffer, sizeof(statsBuffer), "Heap: %u bytes, Temp: %u°C", freeHeap, cpuTemp);
    sendMessage(MSG_SYSTEM_STATUS, (uint16_t)(freeHeap / 1024), statsBuffer);
    
    // Vérifier la charge CPU
    uint32_t taskCount = uxTaskGetNumberOfTasks();
    LOG_INFO("TASK", "Nombre de tâches en cours: %u", taskCount);
    
    // Afficher périodiquement le graphique d'utilisation mémoire
    static uint8_t graphCounter = 0;
    if (++graphCounter >= 5) {
      graphCounter = 0;
      logMemoryGraph();
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
