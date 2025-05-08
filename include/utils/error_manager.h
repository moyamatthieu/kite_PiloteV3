#ifndef ERROR_MANAGER_H
#define ERROR_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <string>
#include <map>

/**
 * Codes d'erreur standardisés pour tout le système
 */
enum class ErrorCode {
    SUCCESS = 0,          // Aucune erreur
    
    // Erreurs générales (1-99)
    UNKNOWN_ERROR = 1,    // Erreur inconnue
    TIMEOUT = 2,          // Délai d'attente dépassé
    INVALID_STATE = 3,    // État système invalide
    INVALID_PARAMETER = 4, // Paramètre invalide
    
    // Erreurs matérielles (100-199)
    HARDWARE_FAILURE = 100, // Défaillance matérielle générique
    SENSOR_ERROR = 101,    // Erreur de capteur
    ACTUATOR_ERROR = 102,  // Erreur d'actionneur
    SERVO_ERROR = 103,     // Erreur de servomoteur
    WINCH_ERROR = 104,     // Erreur de treuil
    IMU_ERROR = 105,       // Erreur d'IMU
    DISPLAY_ERROR = 106,   // Erreur d'affichage
    
    // Erreurs de communication (200-299)
    COMMUNICATION_ERROR = 200, // Erreur de communication générique
    WIFI_CONNECTION_ERROR = 201, // Erreur de connexion WiFi
    NETWORK_TIMEOUT = 202,     // Timeout réseau
    SERVER_ERROR = 203,        // Erreur du serveur web
    API_ERROR = 204,           // Erreur d'API
    
    // Erreurs de ressources (300-399)
    RESOURCE_ERROR = 300,     // Erreur de ressource générique
    OUT_OF_MEMORY = 301,      // Mémoire insuffisante
    FILE_ERROR = 302,         // Erreur de fichier
    
    // Erreurs de configuration (400-499)
    CONFIG_ERROR = 400,       // Erreur de configuration générique
    INVALID_CONFIG = 401,     // Configuration invalide
    
    // Erreurs d'exécution (500-599)
    RUNTIME_ERROR = 500,      // Erreur d'exécution générique
    TASK_CREATION_ERROR = 501, // Erreur de création de tâche
    MUTEX_ERROR = 502,        // Erreur de mutex
};

/**
 * Niveaux de sévérité pour les erreurs
 */
enum class ErrorSeverity {
    INFO = 0,           // Information seulement, pas une erreur
    LOW_SEVERITY = 1,   // Erreur mineure, pas besoin d'action immédiate
    MEDIUM = 2,         // Erreur significative, action recommandée
    HIGH_SEVERITY = 3,  // Erreur grave nécessitant une action immédiate
    CRITICAL = 4        // Erreur critique mettant en danger le système
};

/**
 * Stratégies de récupération après erreur
 */
enum class RecoveryStrategy {
    NONE,               // Pas de récupération automatique
    RETRY,              // Réessayer l'opération
    REINITIALIZE,       // Réinitialiser le composant
    FALLBACK,           // Utiliser une alternative
    RESET_COMPONENT,    // Réinitialiser le composant spécifique
    RESET_SYSTEM,       // Redémarrer le système
    SAFE_MODE           // Passer en mode sécurisé
};

/**
 * Structure détaillée pour les erreurs
 */
struct ErrorDetails {
    ErrorCode code;                 // Code d'erreur
    ErrorSeverity severity;         // Sévérité de l'erreur
    const char* module;             // Module source de l'erreur
    const char* description;        // Description de l'erreur
    RecoveryStrategy strategy;      // Stratégie de récupération recommandée
    unsigned long timestamp;        // Timestamp de l'erreur
    uint32_t count;                 // Nombre d'occurrences
    bool resolved;                  // L'erreur a-t-elle été résolue?
};

/**
 * Gestionnaire d'erreurs centralisé
 * Implémente un pattern Singleton pour assurer une instance unique
 */
class ErrorManager {
private:
    // Instance singleton
    static ErrorManager* instance;
    
    // Historique des erreurs récentes
    std::vector<ErrorDetails> errorHistory;
    
    // Nombre maximum d'erreurs à conserver dans l'historique
    static const uint8_t MAX_ERROR_HISTORY = 20;
    
    // Mutex pour la protection des accès concurrents
    SemaphoreHandle_t errorMutex;
    
    // Compteur d'erreurs par code
    uint32_t errorCounts[static_cast<int>(ErrorCode::TASK_CREATION_ERROR) + 1];
    
    // Callback pour les erreurs critiques
    void (*criticalErrorCallback)(const ErrorDetails& error);
    
    // Constructeur privé (pattern Singleton)
    ErrorManager();
    
    // Stratégies de récupération pour chaque code d'erreur
    RecoveryStrategy recoveryStrategies[static_cast<int>(ErrorCode::TASK_CREATION_ERROR) + 1];

    std::map<ErrorCode, std::string> errorMessages;

public:
    // Destructeur
    ~ErrorManager();
    
    // Point d'accès pour l'instance Singleton
    static ErrorManager* getInstance();
    
    /**
     * Signale une erreur avec tous les détails
     * @param code Code d'erreur
     * @param severity Sévérité de l'erreur
     * @param module Module source de l'erreur
     * @param description Description de l'erreur
     * @param strategy Stratégie de récupération (optionnel)
     * @return true si l'erreur a été correctement signalée
     */
    bool reportError(ErrorCode code, ErrorSeverity severity, const char* module, 
                     const char* description, RecoveryStrategy strategy = RecoveryStrategy::NONE);
    
    /**
     * Version simplifiée pour signaler une erreur
     * @param code Code d'erreur
     * @param module Module source de l'erreur
     * @param description Description de l'erreur
     * @return true si l'erreur a été correctement signalée
     */
    bool reportError(ErrorCode code, const char* module, const char* description);
    
    /**
     * Tente une récupération automatique pour une erreur
     * @param code Code d'erreur pour lequel tenter une récupération
     * @return true si la récupération a réussi
     */
    bool attemptRecovery(ErrorCode code);
    
    /**
     * Définit une stratégie de récupération pour un code d'erreur spécifique
     * @param code Code d'erreur
     * @param strategy Stratégie de récupération
     */
    void setRecoveryStrategy(ErrorCode code, RecoveryStrategy strategy);
    
    /**
     * Définit un callback pour les erreurs critiques
     * @param callback Fonction à appeler lors d'une erreur critique
     */
    void setCriticalErrorCallback(void (*callback)(const ErrorDetails& error));
    
    /**
     * Récupère l'historique des erreurs récentes
     * @return Vecteur contenant les erreurs récentes
     */
    std::vector<ErrorDetails> getErrorHistory();
    
    /**
     * Récupère le nombre d'occurrences d'un code d'erreur spécifique
     * @param code Code d'erreur
     * @return Nombre d'occurrences
     */
    uint32_t getErrorCount(ErrorCode code);
    
    /**
     * Marque une erreur comme résolue
     * @param code Code d'erreur à marquer comme résolu
     * @param module Module concerné (optionnel, pour préciser)
     * @return true si au moins une erreur a été marquée comme résolue
     */
    bool resolveError(ErrorCode code, const char* module = nullptr);
    
    /**
     * Efface l'historique des erreurs
     */
    void clearErrorHistory();
    
    /**
     * Journalise toutes les erreurs non résolues
     */
    void logUnresolvedErrors();

    const char* getErrorMessage(ErrorCode code) {
        return errorMessages[code].c_str();
    }
};

#endif // ERROR_MANAGER_H