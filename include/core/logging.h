/*
  -----------------------
  Kite PiloteV3 - Module de journalisation
  -----------------------
  
  Système de journalisation avancé pour le suivi des erreurs et le débogage.
  
  Version: 2.1.0
  Date: 15 Mai 2024
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef UTILS_LOGGING_H
#define UTILS_LOGGING_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h> // Pour les sémaphores/mutex
#include "../common/global_enums.h" // Pour LogLevel

// Niveau de journalisation global
// Peut être modifié à l'exécution
extern LogLevel currentLogLevel;

// Macros pour faciliter la journalisation
#define LOG_ERROR(tag, format, ...) LoggingModule::getInstance()->print(LogLevel::ERROR, tag, format, ##__VA_ARGS__)
#define LOG_WARNING(tag, format, ...) LoggingModule::getInstance()->print(LogLevel::WARNING, tag, format, ##__VA_ARGS__)
#define LOG_INFO(tag, format, ...) LoggingModule::getInstance()->print(LogLevel::INFO, tag, format, ##__VA_ARGS__)
#define LOG_DEBUG(tag, format, ...) LoggingModule::getInstance()->print(LogLevel::DEBUG, tag, format, ##__VA_ARGS__)

/**
 * Classe LoggingModule - Implémente le système de journalisation
 * 
 * Cette classe suit le pattern Singleton pour garantir une instance unique
 * du système de journalisation dans toute l'application.
 * Elle ne dérive pas de ManagedComponent car elle est un utilitaire de bas niveau.
 */
class LoggingModule {
private:
    // Instance unique (pattern Singleton)
    static LoggingModule* _instance;
    SemaphoreHandle_t _logMutex; // Mutex pour la journalisation
    
    // Constructeur privé (pattern Singleton)
    LoggingModule();
    
    // État d'initialisation
    bool _initialized;
    
    // Timestamp de démarrage pour calculer le temps relatif
    unsigned long _startTime;
    unsigned long _logMessageId;
    
    // Buffer pour les messages
    char* _logBuffer;
    
    // Historique de l'utilisation de la mémoire
    uint32_t* _freeHeapHistory;
    uint32_t* _minFreeHeapHistory;
    int _historyIndex;
    bool _historyFilled;
    
    // Noms des niveaux de journalisation (correspondant à l'enum LogLevel)
    const char** _levelNames;
    
    // Couleurs ANSI
    const char** _colors;
    const char* _colorReset;

public:
    /**
     * Obtient l'instance unique du module de journalisation
     * @return Instance du LoggingModule
     */
    static LoggingModule* getInstance();
    
    /**
     * Initialise le système de journalisation
     * @param level Niveau de journalisation initial de type LogLevel
     * @param baudRate Vitesse de la communication série
     * @return true si l'initialisation a réussi, false sinon
     */
    bool init(LogLevel level, unsigned long baudRate = 115200);
    
    /**
     * Définit le niveau de journalisation
     * @param level Nouveau niveau de journalisation de type LogLevel
     */
    void setLevel(LogLevel level);
    
    /**
     * Obtient le niveau de journalisation actuel
     * @return Niveau de journalisation actuel de type LogLevel
     */
    LogLevel getLevel() const;
    
    /**
     * Affiche un message de journalisation
     * @param level Niveau de journalisation du message de type LogLevel
     * @param tag Étiquette identifiant la source du message
     * @param format Format du message (comme printf)
     * @param ... Arguments variables pour le format
     */
    void print(LogLevel level, const char* tag, const char* format, ...);
    
    /**
     * Affiche un message de journalisation formaté avec va_list
     * @param level Niveau de journalisation du message de type LogLevel
     * @param tag Étiquette identifiant la source du message
     * @param format Format du message (comme printf)
     * @param args Liste d'arguments variables
     */
    void vprint(LogLevel level, const char* tag, const char* format, va_list args);
    
    /**
     * Obtient une représentation texte d'un niveau de journalisation
     * @param level Niveau de journalisation de type LogLevel
     * @return Chaîne représentant le niveau
     */
    const char* levelToString(LogLevel level) const;
    
    /**
     * Journalise l'utilisation de la mémoire
     * @param tag Étiquette identifiant la source du message
     */
    void logMemoryUsage(const char* tag);
    
    /**
     * Affiche un graphique ASCII de l'utilisation de la mémoire au fil du temps
     * Utilise les données collectées par logMemoryUsage
     */
    void logMemoryGraph();
    
    /**
     * Vérifie si le module est initialisé
     * @return true si le module est initialisé, false sinon
     */
    bool isInitialized() const;
    
    /**
     * Destructeur
     */
    ~LoggingModule();
};

// Fonctions globales pour la compatibilité avec le code existant (utilisant LogLevel enum)

/**
 * Initialise le système de journalisation
 * @param level Niveau de journalisation initial de type LogLevel
 * @param baudRate Vitesse de la communication série
 */
void logInit(LogLevel level, unsigned long baudRate = 115200);

/**
 * Définit le niveau de journalisation
 * @param level Nouveau niveau de journalisation de type LogLevel
 */
void logSetLevel(LogLevel level);

/**
 * Affiche un message de journalisation (utilisé par les macros simplifiées)
 * @param level Niveau de journalisation du message de type LogLevel
 * @param tag Étiquette identifiant la source du message
 * @param format Format du message (comme printf)
 * @param ... Arguments variables pour le format
 */
// La fonction logPrint globale n'est plus nécessaire car les macros appellent directement LoggingModule::getInstance()->print

/**
 * Obtient une représentation texte d'un niveau de journalisation
 * @param level Niveau de journalisation de type LogLevel
 * @return Chaîne représentant le niveau
 */
const char* logLevelToString(LogLevel level);

/**
 * Journalise l'utilisation de la mémoire
 * @param tag Étiquette identifiant la source du message
 */
void logMemoryUsage(const char* tag);

/**
 * Affiche un graphique ASCII de l'utilisation de la mémoire au fil du temps
 * Utilise les données collectées par logMemoryUsage
 */
void logMemoryGraph();

#endif // UTILS_LOGGING_H
