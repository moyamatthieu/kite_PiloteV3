#ifndef GLOBAL_ENUMS_H
#define GLOBAL_ENUMS_H

#include <stdint.h> // Pour uint8_t
#include <freertos/FreeRTOS.h> // Pour UBaseType_t et configMAX_PRIORITIES

#pragma once

// Énumérations globales pour le projet Kite PiloteV3

// États généraux du système ou des modules
enum class ComponentState {
    UNINITIALIZED,
    INITIALIZING,
    IDLE,
    ACTIVE,
    SUSPENDED,
    ERROR,
    COMPONENT_DISABLED  // Renommé pour éviter le conflit avec la macro DISABLED de esp32-hal-gpio.h
};

// Codes d'erreur généraux
enum class ErrorCode {
    OK = 0,                     // Aucune erreur
    SUCCESS = OK,               // Alias pour OK pour compatibilité
    GENERAL_ERROR,              // Erreur générale
    UNKNOWN_ERROR = GENERAL_ERROR, // Alias pour GENERAL_ERROR pour compatibilité
    TIMEOUT,                    // Délai d'attente dépassé
    NOT_INITIALIZED,            // Composant non initialisé
    ALREADY_INITIALIZED,        // Composant déjà initialisé
    INVALID_STATE,              // État système invalide
    INVALID_ARGUMENT,           // Argument invalide
    INVALID_PARAMETER = INVALID_ARGUMENT, // Alias pour INVALID_ARGUMENT pour compatibilité
    OUT_OF_MEMORY,              // Mémoire insuffisante
    COMPONENT_INITIALIZATION_ERROR, // Erreur d'initialisation de composant
    
    // Spécifiques au matériel (100-199)
    HARDWARE_FAILURE = 100,     // Défaillance matérielle générique
    SENSOR_FAILURE,             // Défaillance d'un capteur
    SENSOR_ERROR = SENSOR_FAILURE, // Alias pour SENSOR_FAILURE pour compatibilité
    ACTUATOR_FAILURE,           // Défaillance d'un actionneur
    ACTUATOR_ERROR = ACTUATOR_FAILURE, // Alias pour ACTUATOR_FAILURE pour compatibilité
    SERVO_ERROR,                // Erreur de servomoteur
    WINCH_ERROR,                // Erreur de treuil
    IMU_ERROR,                  // Erreur d'IMU
    DISPLAY_ERROR,              // Erreur d'affichage
    
    // Spécifiques à la communication (200-299)
    COMMUNICATION_ERROR = 200,  // Erreur de communication générique
    CONNECTION_FAILED,          // Échec de connexion
    WIFI_CONNECTION_ERROR = CONNECTION_FAILED, // Alias pour CONNECTION_FAILED pour compatibilité
    SEND_FAILED,                // Échec d'envoi
    RECEIVE_FAILED,             // Échec de réception
    NETWORK_TIMEOUT,            // Timeout réseau
    SERVER_ERROR,               // Erreur du serveur web
    API_ERROR,                  // Erreur d'API
    
    // Spécifiques aux ressources (300-399)
    RESOURCE_ERROR = 300,       // Erreur de ressource générique
    FILE_ERROR,                 // Erreur de fichier
    
    // Spécifiques à la configuration (400-499)
    CONFIG_ERROR = 400,         // Erreur de configuration générique
    INVALID_CONFIG,             // Configuration invalide
    
    // Spécifiques à l'exécution (500-599)
    RUNTIME_ERROR = 500,        // Erreur d'exécution générique
    TASK_CREATION_ERROR,        // Erreur de création de tâche
    MUTEX_ERROR,                // Erreur de mutex
    
    // ... Autres codes d'erreur peuvent être ajoutés ici
    
    CRITICAL                    // Erreur critique - doit être la dernière valeur pour les tableaux dimensionnés par ErrorCode
};

// Types de messages pour la communication inter-tâches (IPC)
enum class MessageType {
    // Messages système
    SYSTEM_SHUTDOWN_REQUEST,
    SYSTEM_RESTART_REQUEST,
    // Messages des capteurs
    SENSOR_DATA_IMU,
    SENSOR_DATA_WIND,
    SENSOR_DATA_TENSION,
    // Messages de contrôle
    CONTROL_SET_MODE,
    CONTROL_SET_TARGET,
    // Messages UI
    UI_BUTTON_PRESSED,
    UI_UPDATE_DISPLAY,
    // ... autres types de messages
};

// Priorités des tâches FreeRTOS
// (Valeurs plus élevées = priorité plus élevée)
enum class TaskPriority : UBaseType_t {
    LOWEST = 1,
    PRIORITY_LOW = 2,  // Renommé pour éviter conflit avec la macro LOW de esp32-hal-gpio.h
    BELOW_NORMAL = 3,
    NORMAL = 4,
    ABOVE_NORMAL = 5,
    PRIORITY_HIGH = 6,  // Renommé pour éviter conflit avec la macro HIGH de esp32-hal-gpio.h
    HIGHEST = 7,
    REALTIME = configMAX_PRIORITIES - 1
};

// Niveaux de journalisation
enum class LogLevel : uint8_t {
    NONE = 0,
    ERROR = 1,
    WARNING = 2,
    INFO = 3,
    DEBUG = 4
};

#endif // GLOBAL_ENUMS_H

