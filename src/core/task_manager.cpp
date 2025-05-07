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

/* === MODULE TASK MANAGER ===
   Implémentation du gestionnaire de tâches FreeRTOS pour le système Kite PiloteV3.
   Ce module gère la création, la surveillance et l'arrêt des tâches.
*/

#include "../../include/core/task_manager.h"
#include "../../include/utils/logging.h"

// === VARIABLES GLOBALES ===
static TaskDefinition tasks[MAX_TASKS]; // Tableau des tâches gérées
static int taskCount = 0;              // Nombre de tâches ajoutées

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
