/*
  -----------------------
  Kite PiloteV3 - Module de journalisation
  -----------------------
  
  Système de journalisation avancé pour le suivi des erreurs et le débogage.
  
  Version: 2.0.0
  Date: 9 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef UTILS_LOGGING_H
#define UTILS_LOGGING_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h> // Pour les sémaphores/mutex

// Niveaux de journalisation
typedef uint8_t LogLevel;
#define LOG_LEVEL_NONE    0  // Pas de journalisation
#define LOG_LEVEL_ERROR   1  // Erreurs critiques uniquement
#define LOG_LEVEL_WARNING 2  // Erreurs et avertissements
#define LOG_LEVEL_INFO    3  // Informations générales
#define LOG_LEVEL_DEBUG   4  // Messages de débogage détaillés

// Niveau de journalisation global
// Peut être modifié à l'exécution
extern LogLevel currentLogLevel;

// Macros pour faciliter la journalisation
#define LOG_ERROR(tag, format, ...) logPrint(LOG_LEVEL_ERROR, tag, format, ##__VA_ARGS__)
#define LOG_WARNING(tag, format, ...) logPrint(LOG_LEVEL_WARNING, tag, format, ##__VA_ARGS__)
#define LOG_INFO(tag, format, ...) logPrint(LOG_LEVEL_INFO, tag, format, ##__VA_ARGS__)
#define LOG_DEBUG(tag, format, ...) logPrint(LOG_LEVEL_DEBUG, tag, format, ##__VA_ARGS__)

/**
 * Classe LoggingModule - Implémente le système de journalisation
 * 
 * Cette classe suit le pattern Singleton pour garantir une instance unique
 * du système de journalisation dans toute l'application.
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
    unsigned long _logMessageId; // Ajout de l'ID de message
    
    // Buffer pour les messages
    char* _logBuffer;
    
    // Historique de l'utilisation de la mémoire
    uint32_t* _freeHeapHistory;
    uint32_t* _minFreeHeapHistory;
    int _historyIndex;
    bool _historyFilled;
    
    // Noms des niveaux de journalisation
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
     * @param level Niveau de journalisation initial
     * @param baudRate Vitesse de la communication série
     * @return true si l'initialisation a réussi, false sinon
     */
    bool init(LogLevel level, unsigned long baudRate = 115200);
    
    /**
     * Définit le niveau de journalisation
     * @param level Nouveau niveau de journalisation
     */
    void setLevel(LogLevel level);
    
    /**
     * Obtient le niveau de journalisation actuel
     * @return Niveau de journalisation actuel
     */
    LogLevel getLevel() const;
    
    /**
     * Affiche un message de journalisation
     * @param level Niveau de journalisation du message
     * @param tag Étiquette identifiant la source du message
     * @param format Format du message (comme printf)
     * @param ... Arguments variables pour le format
     */
    void print(LogLevel level, const char* tag, const char* format, ...);
    
    /**
     * Affiche un message de journalisation formaté avec va_list
     * @param level Niveau de journalisation du message
     * @param tag Étiquette identifiant la source du message
     * @param format Format du message (comme printf)
     * @param args Liste d'arguments variables
     */
    void vprint(LogLevel level, const char* tag, const char* format, va_list args);
    
    /**
     * Obtient une représentation texte d'un niveau de journalisation
     * @param level Niveau de journalisation
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

// Fonctions globales pour la compatibilité avec le code existant

/**
 * Initialise le système de journalisation
 * @param level Niveau de journalisation initial
 * @param baudRate Vitesse de la communication série
 */
void logInit(LogLevel level, unsigned long baudRate = 115200);

/**
 * Définit le niveau de journalisation
 * @param level Nouveau niveau de journalisation
 */
void logSetLevel(LogLevel level);

/**
 * Affiche un message de journalisation
 * @param level Niveau de journalisation du message
 * @param tag Étiquette identifiant la source du message
 * @param format Format du message (comme printf)
 * @param ... Arguments variables pour le format
 */
void logPrint(LogLevel level, const char* tag, const char* format, ...);

/**
 * Obtient une représentation texte d'un niveau de journalisation
 * @param level Niveau de journalisation
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
