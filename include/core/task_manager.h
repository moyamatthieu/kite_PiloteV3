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

// Déclarations anticipées
class UIManager;
class WiFiManager;

// Nombre maximum de tâches
#define MAX_TASKS 10

// Structure pour les métriques des tâches
typedef struct {
    uint32_t cpuUsage;
    uint32_t stackHighWaterMark;
    uint32_t lastRunTime;
} TaskStat;

// Structure pour les paramètres de tâches
typedef struct {
    int taskIndex;
    void* manager;
    uint32_t period;
    bool isRealtime;
} TaskParams;

// Structure pour la configuration des tâches
typedef struct {
    const char* name;
    TaskFunction_t function;
    uint32_t stackSize;
    UBaseType_t priority;
    BaseType_t core;
    uint32_t period;
    bool isRealtime;
} TaskConfig;

/**
 * Gestionnaire de tâches multiples
 * Permet de gérer et surveiller les tâches FreeRTOS
 */
class TaskManager {
private:
    // Handles des tâches
    TaskHandle_t displayTaskHandle;
    TaskHandle_t buttonTaskHandle;
    TaskHandle_t wifiMonitorTaskHandle;
    TaskHandle_t systemMonitorTaskHandle;
    
    // Variables d'état
    bool running;
    bool tasksRunning;
    unsigned long lastTaskMetricsTime;
    
    // Handles et statistiques des tâches
    TaskHandle_t taskHandles[MAX_TASKS];
    TaskParams* taskParams[MAX_TASKS];
    TaskStat taskStats[MAX_TASKS];
    
    // Ressources partagées
    static QueueHandle_t messageQueue;
    static SemaphoreHandle_t displayMutex;
    static UIManager* uiManager;
    static WiFiManager* wifiManager;
    
    // Fonctions de tâches
    static void displayTask(void* parameters);
    static void buttonTask(void* parameters);
    static void networkTask(void* parameters);
    static void controlTask(void* parameters);
    static void inputTask(void* parameters);
    static void monitorTask(void* parameters);
    static void sensorTask(void* parameters);
    
public:
    // Constructeur et destructeur
    TaskManager();
    ~TaskManager();
    
    // Initialisation et démarrage
    bool begin(UIManager* ui, WiFiManager* wifi);
    bool startTasks();
    void stopTasks();
    void stopAllTasks();
    
    // Surveillance des tâches
    void updateTaskMetrics();
    bool checkTasksHealth();
    
    // Getters
    bool isRunning() const { return running; }
};

#endif // TASK_MANAGER_H
