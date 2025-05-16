#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "../common/global_enums.h"
#include "logging.h" // Pour journaliser les erreurs

#define MAX_ERROR_MESSAGE_LENGTH 256

struct ErrorInfo {
    ErrorCode code;
    char component[32];
    char message[MAX_ERROR_MESSAGE_LENGTH];
    unsigned long timestamp;
};

class ErrorHandler {
private:
    static ErrorHandler* _instance;
    SemaphoreHandle_t _errorMutex;
    bool _initialized;

    ErrorInfo _lastError;
    uint32_t _errorCount;

    // Constructeur privé
    ErrorHandler();

public:
    // Pas de copie ou d'assignation
    ErrorHandler(const ErrorHandler&) = delete;
    ErrorHandler& operator=(const ErrorHandler&) = delete;

    /**
     * @brief Obtient l'instance unique du ErrorHandler (Singleton).
     * @return Pointeur vers l'instance du ErrorHandler.
     */
    static ErrorHandler* getInstance();

    /**
     * @brief Initialise le gestionnaire d'erreurs.
     * @return true si l'initialisation a réussi, false sinon.
     */
    bool init();

    /**
     * @brief Gère une erreur système.
     * Journalise l'erreur et met à jour l'état de la dernière erreur.
     * @param code Le code d'erreur (ErrorCode).
     * @param component Le nom du composant ou du fichier source de l'erreur.
     * @param format Le message d'erreur formaté (style printf).
     * @param ... Arguments variables pour le formatage du message.
     */
    void handleError(ErrorCode code, const char* component, const char* format, ...);
    
    /**
     * @brief Gère une erreur système avec va_list.
     * @param code Le code d'erreur (ErrorCode).
     * @param component Le nom du composant ou du fichier source de l'erreur.
     * @param format Le message d'erreur formaté (style printf).
     * @param args Liste d'arguments variables.
     */
    void vHandleError(ErrorCode code, const char* component, const char* format, va_list args);

    /**
     * @brief Récupère la dernière erreur enregistrée.
     * @return Une structure ErrorInfo contenant les détails de la dernière erreur.
     */
    ErrorInfo getLastError() const;

    /**
     * @brief Récupère le nombre total d'erreurs enregistrées depuis l'initialisation.
     * @return Le nombre d'erreurs.
     */
    uint32_t getErrorCount() const;

    /**
     * @brief Vérifie si le module est initialisé.
     * @return true si initialisé, false sinon.
     */
    bool isInitialized() const;
    
    /**
     * @brief Destructeur.
     */
    ~ErrorHandler();
};

// Fonction globale pour faciliter le signalement des erreurs
void reportError(ErrorCode code, const char* component, const char* format, ...);

