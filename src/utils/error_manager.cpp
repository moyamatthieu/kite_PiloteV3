/*
  -----------------------
  Kite PiloteV3 - Gestionnaire d'erreurs (Implémentation)
  -----------------------
  
  Implémentation du gestionnaire centralisé des erreurs système.
  
  Version: 3.0.0
  Date: 8 mai 2025
  Auteurs: Équipe Kite PiloteV3
  
  ===== FONCTIONNEMENT =====
  Ce module implémente un gestionnaire d'erreurs qui centralise la gestion,
  le signalement et la récupération des erreurs dans tout le système.
  Il utilise un pattern Singleton et est thread-safe pour l'environnement FreeRTOS.
  
  Principes de fonctionnement :
  1. Centralisation des erreurs avec codes standardisés
  2. Journalisation et suivi des occurrences pour analyses
  3. Stratégies de récupération configurables par type d'erreur
  4. Protection thread-safe avec mutex pour environnement multitâche
  
  Interactions avec d'autres modules :
  - Tous les modules : Signalement des erreurs via ce gestionnaire
  - SystemStateManager : Coordination pour les transitions d'état suite aux erreurs
  - Logging : Journalisation des erreurs
*/

#include "utils/error_manager.h"
#include "utils/logging.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <cstring>

// Initialisation de l'instance singleton
ErrorManager* ErrorManager::instance = nullptr;

/**
 * Constructeur privé (Pattern Singleton)
 */
ErrorManager::ErrorManager() {
    // Création du mutex pour la protection des accès concurrents
    errorMutex = xSemaphoreCreateMutex();
    
    // Initialisation du callback critique
    criticalErrorCallback = nullptr;
    
    // Initialisation des compteurs d'erreurs
    memset(errorCounts, 0, sizeof(errorCounts));
    
    // Initialisation des stratégies de récupération par défaut
    for (int i = 0; i <= static_cast<int>(ErrorCode::TASK_CREATION_ERROR); i++) {
        recoveryStrategies[i] = RecoveryStrategy::NONE;
    }
    
    // Configuration de quelques stratégies de récupération par défaut
    recoveryStrategies[static_cast<int>(ErrorCode::WIFI_CONNECTION_ERROR)] = RecoveryStrategy::RETRY;
    recoveryStrategies[static_cast<int>(ErrorCode::SERVO_ERROR)] = RecoveryStrategy::REINITIALIZE;
    recoveryStrategies[static_cast<int>(ErrorCode::OUT_OF_MEMORY)] = RecoveryStrategy::RESET_SYSTEM;
    
    LOG_INFO("ERROR_MGR", "Gestionnaire d'erreurs initialisé");
}

/**
 * Destructeur
 */
ErrorManager::~ErrorManager() {
    // Libération du mutex
    if (errorMutex != nullptr) {
        vSemaphoreDelete(errorMutex);
        errorMutex = nullptr;
    }
}

/**
 * Point d'accès pour l'instance Singleton
 * @return Pointeur vers l'instance unique du gestionnaire d'erreurs
 */
ErrorManager* ErrorManager::getInstance() {
    // Création de l'instance si elle n'existe pas encore
    if (instance == nullptr) {
        instance = new ErrorManager();
    }
    
    return instance;
}

/**
 * Signale une erreur avec tous les détails
 * @param code Code d'erreur
 * @param severity Sévérité de l'erreur
 * @param module Module source de l'erreur
 * @param description Description de l'erreur
 * @param strategy Stratégie de récupération (optionnel)
 * @return true si l'erreur a été correctement signalée
 */
bool ErrorManager::reportError(ErrorCode code, ErrorSeverity severity, const char* module, 
                               const char* description, RecoveryStrategy strategy) {
    // Vérification des paramètres
    if (module == nullptr || description == nullptr) {
        LOG_ERROR("ERROR_MGR", "Paramètres invalides lors du signalement d'erreur");
        return false;
    }
    
    // Acquisition du mutex
    if (xSemaphoreTake(errorMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        LOG_ERROR("ERROR_MGR", "Impossible d'acquérir le mutex pour signaler une erreur");
        return false;
    }
    
    // Mise à jour du compteur d'erreurs pour ce code
    errorCounts[static_cast<int>(code)]++;
    
    // Création des détails de l'erreur
    ErrorDetails error;
    error.code = code;
    error.severity = severity;
    error.module = module;
    error.description = description;
    error.strategy = (strategy == RecoveryStrategy::NONE) ? recoveryStrategies[static_cast<int>(code)] : strategy;
    error.timestamp = millis();
    error.count = errorCounts[static_cast<int>(code)];
    error.resolved = false;
    
    // Ajout à l'historique des erreurs (avec limitation de taille)
    if (errorHistory.size() >= MAX_ERROR_HISTORY) {
        errorHistory.erase(errorHistory.begin());
    }
    errorHistory.push_back(error);
    
    // Libération du mutex
    xSemaphoreGive(errorMutex);
    
    // Journalisation de l'erreur avec le niveau approprié
    const char* severityStr = "INCONNU";
    switch (severity) {
        case ErrorSeverity::INFO:
            severityStr = "INFO";
            LOG_INFO("ERROR", "[%s] %s: %s (Code: %d, Count: %lu)", 
                    module, severityStr, description, static_cast<int>(code), error.count);
            break;
        case ErrorSeverity::LOW_SEVERITY:
            severityStr = "LOW";
            LOG_WARNING("ERROR", "[%s] %s: %s (Code: %d, Count: %lu)", 
                      module, severityStr, description, static_cast<int>(code), error.count);
            break;
        case ErrorSeverity::MEDIUM:
            severityStr = "MEDIUM";
            LOG_WARNING("ERROR", "[%s] %s: %s (Code: %d, Count: %lu)", 
                      module, severityStr, description, static_cast<int>(code), error.count);
            break;
        case ErrorSeverity::HIGH_SEVERITY:
            severityStr = "HIGH";
            LOG_ERROR("ERROR", "[%s] %s: %s (Code: %d, Count: %lu)", 
                    module, severityStr, description, static_cast<int>(code), error.count);
            break;
        case ErrorSeverity::CRITICAL:
            severityStr = "CRITICAL";
            LOG_ERROR("ERROR", "[%s] %s: %s (Code: %d, Count: %lu)", 
                    module, severityStr, description, static_cast<int>(code), error.count);
            
            // Appel du callback pour les erreurs critiques si défini
            if (criticalErrorCallback != nullptr) {
                criticalErrorCallback(error);
            }
            break;
    }
    
    // Tentative de récupération automatique si une stratégie est définie
    if (error.strategy != RecoveryStrategy::NONE) {
        attemptRecovery(code);
    }
    
    return true;
}

/**
 * Version simplifiée pour signaler une erreur
 * @param code Code d'erreur
 * @param module Module source de l'erreur
 * @param description Description de l'erreur
 * @return true si l'erreur a été correctement signalée
 */
bool ErrorManager::reportError(ErrorCode code, const char* module, const char* description) {
    // Déterminer la sévérité en fonction du code d'erreur
    ErrorSeverity severity;
    if (code == ErrorCode::SUCCESS) {
        severity = ErrorSeverity::INFO;
    } else if (static_cast<int>(code) < 100) {
        severity = ErrorSeverity::LOW_SEVERITY;
    } else if (static_cast<int>(code) < 200) {
        severity = ErrorSeverity::MEDIUM;
    } else if (static_cast<int>(code) < 300) {
        severity = ErrorSeverity::HIGH_SEVERITY;
    } else {
        severity = ErrorSeverity::CRITICAL;
    }
    
    return reportError(code, severity, module, description);
}

/**
 * Tente une récupération automatique pour une erreur
 * @param code Code d'erreur pour lequel tenter une récupération
 * @return true si la récupération a réussi
 */
bool ErrorManager::attemptRecovery(ErrorCode code) {
    bool success = false;
    RecoveryStrategy strategy;
    
    // Acquisition du mutex
    if (xSemaphoreTake(errorMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        LOG_ERROR("ERROR_MGR", "Impossible d'acquérir le mutex pour la récupération");
        return false;
    }
    
    // Récupération de la stratégie configurée
    strategy = recoveryStrategies[static_cast<int>(code)];
    
    // Libération du mutex
    xSemaphoreGive(errorMutex);
    
    // Implémentation des stratégies de récupération
    switch (strategy) {
        case RecoveryStrategy::NONE:
            LOG_INFO("ERROR_MGR", "Aucune stratégie de récupération définie pour le code %d", static_cast<int>(code));
            return false;
            
        case RecoveryStrategy::RETRY:
            LOG_INFO("ERROR_MGR", "Tentative de réessai pour l'erreur %d", static_cast<int>(code));
            // La logique de réessai dépend du contexte et sera implémentée par le module appelant
            success = true;
            break;
            
        case RecoveryStrategy::REINITIALIZE:
            LOG_INFO("ERROR_MGR", "Réinitialisation du composant pour l'erreur %d", static_cast<int>(code));
            // La réinitialisation dépend du contexte et sera implémentée par le module appelant
            success = true;
            break;
            
        case RecoveryStrategy::FALLBACK:
            LOG_INFO("ERROR_MGR", "Utilisation d'une alternative pour l'erreur %d", static_cast<int>(code));
            // L'alternative dépend du contexte et sera implémentée par le module appelant
            success = true;
            break;
            
        case RecoveryStrategy::RESET_COMPONENT:
            LOG_INFO("ERROR_MGR", "Réinitialisation du composant pour l'erreur %d", static_cast<int>(code));
            // La réinitialisation du composant dépend du contexte
            success = true;
            break;
            
        case RecoveryStrategy::RESET_SYSTEM:
            LOG_WARNING("ERROR_MGR", "Redémarrage du système pour l'erreur %d", static_cast<int>(code));
            // Dans une implémentation réelle, cela pourrait appeler ESP.restart()
            success = true;
            break;
            
        case RecoveryStrategy::SAFE_MODE:
            LOG_WARNING("ERROR_MGR", "Passage en mode sécurisé pour l'erreur %d", static_cast<int>(code));
            // Dans une implémentation réelle, cela pourrait configurer le système en mode minimal
            success = true;
            break;
            
        default:
            LOG_ERROR("ERROR_MGR", "Stratégie de récupération inconnue: %d", static_cast<int>(strategy));
            success = false;
            break;
    }
    
    // Si la récupération a réussi, marquer l'erreur comme résolue
    if (success) {
        resolveError(code);
    }
    
    return success;
}

/**
 * Définit une stratégie de récupération pour un code d'erreur spécifique
 * @param code Code d'erreur
 * @param strategy Stratégie de récupération
 */
void ErrorManager::setRecoveryStrategy(ErrorCode code, RecoveryStrategy strategy) {
    // Acquisition du mutex
    if (xSemaphoreTake(errorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        recoveryStrategies[static_cast<int>(code)] = strategy;
        xSemaphoreGive(errorMutex);
        
        LOG_INFO("ERROR_MGR", "Stratégie de récupération pour le code %d définie à %d", 
                static_cast<int>(code), static_cast<int>(strategy));
    } else {
        LOG_ERROR("ERROR_MGR", "Impossible d'acquérir le mutex pour définir la stratégie de récupération");
    }
}

/**
 * Définit un callback pour les erreurs critiques
 * @param callback Fonction à appeler lors d'une erreur critique
 */
void ErrorManager::setCriticalErrorCallback(void (*callback)(const ErrorDetails& error)) {
    // Acquisition du mutex
    if (xSemaphoreTake(errorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        criticalErrorCallback = callback;
        xSemaphoreGive(errorMutex);
        
        LOG_INFO("ERROR_MGR", "Callback pour erreurs critiques défini");
    } else {
        LOG_ERROR("ERROR_MGR", "Impossible d'acquérir le mutex pour définir le callback d'erreur critique");
    }
}

/**
 * Récupère l'historique des erreurs récentes
 * @return Vecteur contenant les erreurs récentes
 */
std::vector<ErrorDetails> ErrorManager::getErrorHistory() {
    std::vector<ErrorDetails> history;
    
    // Acquisition du mutex
    if (xSemaphoreTake(errorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        history = errorHistory;
        xSemaphoreGive(errorMutex);
    } else {
        LOG_ERROR("ERROR_MGR", "Impossible d'acquérir le mutex pour récupérer l'historique des erreurs");
    }
    
    return history;
}

/**
 * Récupère le nombre d'occurrences d'un code d'erreur spécifique
 * @param code Code d'erreur
 * @return Nombre d'occurrences
 */
uint32_t ErrorManager::getErrorCount(ErrorCode code) {
    uint32_t count = 0;
    
    // Acquisition du mutex
    if (xSemaphoreTake(errorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        count = errorCounts[static_cast<int>(code)];
        xSemaphoreGive(errorMutex);
    } else {
        LOG_ERROR("ERROR_MGR", "Impossible d'acquérir le mutex pour récupérer le compteur d'erreurs");
    }
    
    return count;
}

/**
 * Marque une erreur comme résolue
 * @param code Code d'erreur à marquer comme résolu
 * @param module Module concerné (optionnel, pour préciser)
 * @return true si au moins une erreur a été marquée comme résolue
 */
bool ErrorManager::resolveError(ErrorCode code, const char* module) {
    bool resolved = false;
    
    // Acquisition du mutex
    if (xSemaphoreTake(errorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Parcours de l'historique des erreurs
        for (auto& error : errorHistory) {
            if (error.code == code && !error.resolved && 
                (module == nullptr || strcmp(error.module, module) == 0)) {
                error.resolved = true;
                resolved = true;
                
                LOG_INFO("ERROR_MGR", "Erreur %d du module %s marquée comme résolue", 
                        static_cast<int>(code), error.module);
            }
        }
        
        xSemaphoreGive(errorMutex);
    } else {
        LOG_ERROR("ERROR_MGR", "Impossible d'acquérir le mutex pour résoudre l'erreur");
    }
    
    return resolved;
}

/**
 * Efface l'historique des erreurs
 */
void ErrorManager::clearErrorHistory() {
    // Acquisition du mutex
    if (xSemaphoreTake(errorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        errorHistory.clear();
        xSemaphoreGive(errorMutex);
        
        LOG_INFO("ERROR_MGR", "Historique des erreurs effacé");
    } else {
        LOG_ERROR("ERROR_MGR", "Impossible d'acquérir le mutex pour effacer l'historique des erreurs");
    }
}

/**
 * Journalise toutes les erreurs non résolues
 */
void ErrorManager::logUnresolvedErrors() {
    // Acquisition du mutex
    if (xSemaphoreTake(errorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        uint32_t count = 0;
        
        LOG_INFO("ERROR_MGR", "Liste des erreurs non résolues:");
        
        for (const auto& error : errorHistory) {
            if (!error.resolved) {
                count++;
                LOG_INFO("ERROR_MGR", "%lu: [%s] %s (Code: %d, Sévérité: %d, Temps: %lu)", 
                        count, error.module, error.description, 
                        static_cast<int>(error.code), static_cast<int>(error.severity),
                        error.timestamp);
            }
        }
        
        if (count == 0) {
            LOG_INFO("ERROR_MGR", "Aucune erreur non résolue");
        }
        
        xSemaphoreGive(errorMutex);
    } else {
        LOG_ERROR("ERROR_MGR", "Impossible d'acquérir le mutex pour journaliser les erreurs non résolues");
    }
}