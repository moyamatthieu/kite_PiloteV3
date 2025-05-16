/*
  -----------------------
  Kite PiloteV3 - Module de Gestion des Tâches (Interface)
  -----------------------
  
  Interface du gestionnaire de tâches FreeRTOS pour le système Kite PiloteV3.
  
  Version: 3.0.0
  Date: 7 mai 2025
  Auteurs: Équipe Kite PiloteV3
  
  ===== INTERFACE PUBLIQUE =====
  Ce fichier définit l'interface publique pour le gestionnaire de tâches qui
  orchestre l'exécution des différentes fonctionnalités du système.
  
  Principales fonctionnalités exposées :
  - TaskManager::begin() : Initialisation du gestionnaire et de ses ressources
  - TaskManager::startManagedTasks() : Création et démarrage des tâches des composants gérés
  - TaskManager::stopManagedTasks() : Arrêt des tâches des composants gérés
  - TaskManager::stopAll() : Arrêt de toutes les tâches et nettoyage des ressources
  
  Interactions avec d'autres modules :
  - ManagedComponent : Gestion des composants gérés dynamiquement
  
  Contraintes techniques :
  - Les tâches ont des priorités différentes (1-24, 24 étant la plus élevée)
  - Les tailles de pile doivent être définies avec soin selon les besoins de chaque tâche
  - Les délais doivent utiliser vTaskDelayUntil plutôt que vTaskDelay pour une meilleure précision
*/

#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "core/component.h" // Pour utiliser ManagedComponent
#include "core/logging.h"  // Pour LOG_INFO, LOG_ERROR
#include "core/config.h"    // Pour CONFIG_CORE_DEFAULTS, etc.
#include <Arduino.h>
#include <freertos/queue.h>
#include "component.h" // Correction de l'inclusion
#include "../common/global_enums.h" // Correction du chemin d'inclusion

// === CLASSE TASK MANAGER ===
/**
 * Gestionnaire de tâches multiples
 * Permet de gérer et surveiller les tâches FreeRTOS associées aux composants gérés
 */
class TaskManager {
private:
    std::vector<ManagedComponent*> components; // Liste des composants gérés
    bool running;
    bool tasksRunning; // Indique si les tâches des composants sont démarrées

    // La file de messages et le mutex d'affichage pourraient être gérés ailleurs à terme.
    static QueueHandle_t systemMessageQueue; // Renommé pour plus de clarté
    static SemaphoreHandle_t displayMutex;   // Conservé pour l'instant

    // La tâche de monitoring est conservée pour l'instant, mais pourrait évoluer.
    static TaskHandle_t systemMonitorTaskHandle;
    static void systemMonitorTask(void* parameters);

public:
    TaskManager();
    ~TaskManager();

    // Initialise le gestionnaire et enregistre les composants.
    bool begin(const std::vector<ManagedComponent*>& managedComponents);
    
    // Démarre les tâches associées aux composants gérés.
    bool startManagedTasks();
    
    // Arrête les tâches associées aux composants gérés.
    void stopManagedTasks();
    
    // Arrête toutes les tâches et nettoie les ressources.
    void stopAll(); 

    bool isRunning() const { return running; }
    bool areTasksRunning() const { return tasksRunning; }

    // Méthodes statiques pour accéder aux ressources partagées (si nécessaire)
    static QueueHandle_t getSystemMessageQueue();
    static SemaphoreHandle_t getDisplayMutex(); // Si un accès direct est toujours requis
};

#endif // TASK_MANAGER_H
