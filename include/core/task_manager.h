/*
  -----------------------
  Kite PiloteV3 - Module de gestion des tâches
  -----------------------
  
  Module de gestion des tâches multiples et parallèles pour le système Kite PiloteV3.
  
  Version: 1.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "../include/config.h"
#include "../hardware/io/ui_manager.h"
#include "../communication/wifi_manager.h"

// Types de messages
enum MessageType {
    MSG_NONE = 0,
    MSG_DISPLAY_UPDATE,
    MSG_TOUCH_EVENT,
    MSG_WIFI_STATUS,
    MSG_SYSTEM_STATUS,
    MSG_ERROR,
    MSG_TASK_LIST
};

// Structure des messages
typedef struct {
    uint8_t type;
    uint16_t value;
    String message;
} TaskMessage;

class TaskManager {
public:
    TaskManager();
    ~TaskManager();
    
    // Initialisation
    void begin(UIManager* ui, WiFiManager* wifi);
    
    // Gestion des tâches
    void startTasks();
    void stopTasks();
    
    // Communication
    static bool sendMessage(MessageType type, uint16_t value, const char* message);
    
    // État
    bool isRunning() const;
    
private:
    // Handles des tâches
    TaskHandle_t displayTaskHandle;
    TaskHandle_t buttonTaskHandle;
    TaskHandle_t wifiMonitorTaskHandle;
    TaskHandle_t systemMonitorTaskHandle;
    
    // File d'attente et mutex
    static QueueHandle_t messageQueue;
    static SemaphoreHandle_t displayMutex;
    
    // Références aux gestionnaires
    static UIManager* uiManager;
    static WiFiManager* wifiManager;
    
    // État
    bool running;
    
    // Tâches FreeRTOS
    static void displayTask(void* parameter);
    static void buttonTask(void* parameter);
    static void wifiMonitorTask(void* parameter);
    static void systemMonitorTask(void* parameter);
    
    // Traitement des messages
    static void processMessage(const TaskMessage& msg);
};

#endif // TASK_MANAGER_H
