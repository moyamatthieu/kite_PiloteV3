/*
  -----------------------
  Kite PiloteV3 - Module de journalisation (Implémentation)
  -----------------------
  
  Implémentation du système de journalisation avancé, optimisé pour les performances.
  
  Version: 2.0.0
  Date: 9 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "../../include/utils/logging.h"
#include "../../include/core/config.h"
#include <stdarg.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>  // Pour accéder à pcTaskGetName

// Définition de l'instance singleton
LoggingModule* LoggingModule::_instance = nullptr;

// Variable globale pour le niveau de journalisation actuel
LogLevel currentLogLevel = LOG_LEVEL_INFO; // Niveau par défaut

// Tailles des buffers
#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE 192
#endif

#ifndef MEM_HISTORY_SIZE
#define MEM_HISTORY_SIZE 30
#endif

// Noms des niveaux de journalisation pour l'affichage
static const char* LOG_LEVEL_NAMES[] = {
  "NONE",
  "ERROR",
  "WARNING",
  "INFO",
  "DEBUG"
};

// Couleurs ANSI pour le terminal
#if MEMORY_OPTIMIZATION_ENABLED
static const char* LOG_COLORS[] = {
  "",           // NONE - pas de couleur
  "E",          // ERROR - version courte
  "W",          // WARNING - version courte
  "I",          // INFO - version courte
  "D"           // DEBUG - version courte
};
static const char* logColorReset = "";
#else
static const char* LOG_COLORS[] = {
  "",           // NONE - pas de couleur
  "\033[31m",   // ERROR - rouge
  "\033[33m",   // WARNING - jaune
  "\033[32m",   // INFO - vert
  "\033[36m"    // DEBUG - cyan
};
static const char* logColorReset = "\033[0m"; // Renommé pour éviter les conflits
#endif

// Constructeur privé
LoggingModule::LoggingModule() : 
    _initialized(false),
    _startTime(0),
    _logMessageId(0), // Initialisation de _logMessageId
    _historyIndex(0),
    _historyFilled(false) {
    
    // Création du Mutex pour la journalisation
    _logMutex = xSemaphoreCreateMutex();
    if (_logMutex == NULL) {
        // Gérer l'erreur de création du mutex, peut-être logger sur Serial directement
        // si Serial est déjà initialisé, ou marquer un état d'erreur.
        // Pour l'instant, on ne fait rien de spécial, mais c'est important en production.
    }

    // Allocation des buffers
    _logBuffer = new char[LOG_BUFFER_SIZE];
    _freeHeapHistory = new uint32_t[MEM_HISTORY_SIZE];
    _minFreeHeapHistory = new uint32_t[MEM_HISTORY_SIZE];
    
    // Initialisation des tableaux
    for (int i = 0; i < MEM_HISTORY_SIZE; i++) {
        _freeHeapHistory[i] = 0;
        _minFreeHeapHistory[i] = 0;
    }
    
    // Copie des références aux tableaux statiques
    _levelNames = LOG_LEVEL_NAMES;
    _colors = LOG_COLORS;
    _colorReset = logColorReset;
}

// Destructeur
LoggingModule::~LoggingModule() {
    // Libération des ressources
    if (_logBuffer) {
        delete[] _logBuffer;
        _logBuffer = nullptr;
    }
    
    if (_freeHeapHistory) {
        delete[] _freeHeapHistory;
        _freeHeapHistory = nullptr;
    }
    
    if (_minFreeHeapHistory) {
        delete[] _minFreeHeapHistory;
        _minFreeHeapHistory = nullptr;
    }

    // Suppression du mutex
    if (_logMutex != NULL) {
        vSemaphoreDelete(_logMutex);
        _logMutex = NULL;
    }
}

// Obtention de l'instance unique
LoggingModule* LoggingModule::getInstance() {
    // Création de l'instance si elle n'existe pas encore
    if (_instance == nullptr) {
        _instance = new LoggingModule();
    }
    return _instance;
}

// Initialisation du système de journalisation
bool LoggingModule::init(LogLevel level, unsigned long baudRate) {
    if (_initialized) {
        return true; // Déjà initialisé
    }
    
    // Initialisation du port série si ce n'est pas déjà fait
    if (!Serial) {
        Serial.begin(baudRate);
        #if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
        unsigned long startWait = millis();
        while (!Serial && (millis() - startWait < 1000)) {
            delay(10); // Wait for Serial to initialize with timeout
        }
        #endif
    }
    
    // Initialisation du timestamp de démarrage
    _startTime = millis();
    
    // Définition du niveau de journalisation
    currentLogLevel = level;
    
    _initialized = true;
    
    // Message d'initialisation
    Serial.printf("\033[32m[INFO] [LOGGING] Initialisation du système de log au niveau %d\033[0m\n", level);
    
    return true;
}

// Définition du niveau de journalisation
void LoggingModule::setLevel(LogLevel level) {
    currentLogLevel = level;
    const char* levelName = (level >= LOG_LEVEL_NONE && level <= LOG_LEVEL_DEBUG)
                         ? _levelNames[level] 
                         : "UNKNOWN";
    Serial.printf("\033[32m[INFO] [LOG] Niveau de journalisation changé: %s\033[0m\n", levelName);
}

// Obtention du niveau de journalisation actuel
LogLevel LoggingModule::getLevel() const {
    return currentLogLevel;
}

// Conversion d'un niveau de journalisation en chaîne
const char* LoggingModule::levelToString(LogLevel level) const {
    if (level >= LOG_LEVEL_NONE && level <= LOG_LEVEL_DEBUG) {
        return _levelNames[level];
    }
    return "UNKNOWN";
}

// Affichage d'un message de journalisation
void LoggingModule::print(LogLevel level, const char* tag, const char* format, ...) {
    if (level > currentLogLevel || level == LOG_LEVEL_NONE || !_initialized) {
        return;
    }

    if (_logMutex != NULL) {
        if (xSemaphoreTake(_logMutex, portMAX_DELAY) == pdTRUE) {
            va_list args;
            va_start(args, format);
            vprint(level, tag, format, args); // Appel à la nouvelle méthode vprint
            va_end(args);
            xSemaphoreGive(_logMutex);
        }
    } else {
        // Fallback si le mutex n'a pas été créé (ne devrait pas arriver si bien initialisé)
        va_list args;
        va_start(args, format);
        vprint(level, tag, format, args);
        va_end(args);
    }
}

// Nouvelle méthode privée pour le formatage et l'impression réels
void LoggingModule::vprint(LogLevel level, const char* tag, const char* format, va_list args) {
    _logMessageId++; // Incrémentation de l'ID du message

    // Récupérer le nom de la tâche en cours
    char taskName[configMAX_TASK_NAME_LEN] = "main"; // Utilisation de configMAX_TASK_NAME_LEN
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();
        if (currentTask != NULL) {
            const char* name = pcTaskGetName(currentTask);
            if (name != NULL) {
                strncpy(taskName, name, sizeof(taskName) - 1);
                taskName[sizeof(taskName) - 1] = '\0'; // Assurer la terminaison de la chaîne
            }
        }
    }
    
#if MEMORY_OPTIMIZATION_ENABLED
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - _startTime;
    size_t headerLength = snprintf(_logBuffer, LOG_BUFFER_SIZE, 
                   "[ID:%lu][%lu.%03u] %s %s [%s]: ", // Ajout de l'ID du message
                   _logMessageId, // Utilisation de l'ID du message
                   elapsedTime / 1000, elapsedTime % 1000,
                   _levelNames[level], tag, taskName);
    
    if (headerLength >= LOG_BUFFER_SIZE - 1) { // Vérification si le buffer est plein après l'en-tête
        Serial.println("[LOG_ERR] Buffer overflow in header (optimized)");
        return; // Éviter d'écrire plus loin
    }
    
    // Formatage du message utilisateur dans la partie restante du buffer
    vsnprintf(_logBuffer + headerLength, LOG_BUFFER_SIZE - headerLength, format, args);
    Serial.println(_logBuffer); // Imprimer le buffer complet
    
#else // Mode non optimisé (avec couleurs)
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - _startTime;
    unsigned long seconds = elapsedTime / 1000;
    unsigned int milliseconds = elapsedTime % 1000;
    
    size_t headerLength = snprintf(_logBuffer, LOG_BUFFER_SIZE, 
                       "%s[ID:%lu][%6lu.%03u] %7s %-10s [%-*s]%s: ", // Utilisation de configMAX_TASK_NAME_LEN pour taskName
                       _colors[level],
                       _logMessageId, 
                       seconds, milliseconds,
                       _levelNames[level],
                       tag,
                       configMAX_TASK_NAME_LEN -1, // Largeur pour le nom de la tâche
                       taskName,
                       _colorReset); // Renommé en _colorReset dans le constructeur
    
    if (headerLength >= LOG_BUFFER_SIZE - 1) { // Vérification si le buffer est plein après l'en-tête
        Serial.printf("%s[LOG_ERR] Buffer overflow in header (colored)%s\n", _colors[LOG_LEVEL_ERROR], _colorReset);
        return; // Éviter d'écrire plus loin
    }
    
    size_t remainingBuffer = LOG_BUFFER_SIZE - headerLength;
    if (remainingBuffer > 1) { // Au moins 1 caractère + null
        vsnprintf(_logBuffer + headerLength, remainingBuffer, format, args);
    }
    
    Serial.println(_logBuffer); // Imprimer le buffer complet
#endif
}

// Journalisation de l'utilisation de la mémoire
void LoggingModule::logMemoryUsage(const char* tag) {
    if (currentLogLevel < LOG_LEVEL_INFO) {
        return;
    }
    
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();
    uint32_t maxAllocHeap = ESP.getMaxAllocHeap();
    
    static uint8_t skipCounter = 0;
    if (skipCounter++ % 3 == 0) {
        _freeHeapHistory[_historyIndex] = freeHeap;
        _minFreeHeapHistory[_historyIndex] = minFreeHeap;
        _historyIndex = (_historyIndex + 1) % MEM_HISTORY_SIZE;
        if (_historyIndex == 0) {
            _historyFilled = true;
        }
    }
    
#if MEMORY_OPTIMIZATION_ENABLED
    print(LOG_LEVEL_INFO, tag, "Mem: %u/%u (%u%%), Min %u, Max %u",
             freeHeap, totalHeap, (freeHeap * 100) / totalHeap, 
             minFreeHeap, maxAllocHeap);
#else
    print(LOG_LEVEL_INFO, tag, "Mémoire: Libre %u/%u octets (%u%%), Min libre %u, Max bloc %u",
             freeHeap, totalHeap, (freeHeap * 100) / totalHeap, 
             minFreeHeap, maxAllocHeap);
#endif
}

// Affichage d'un graphique de l'utilisation de la mémoire
void LoggingModule::logMemoryGraph() {
#if MEMORY_OPTIMIZATION_ENABLED
  if (currentLogLevel < LOG_LEVEL_INFO) {
        return;
    }
    
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();
    uint32_t maxUsed = ESP.getHeapSize() - minFreeHeap;
    
    Serial.println("--- MÉMOIRE ---");
    Serial.printf("Libre: %u  |  Min libre: %u  |  Max utilisé: %u\n", 
                  freeHeap, minFreeHeap, maxUsed);
#else
    if (currentLogLevel < LOG_LEVEL_INFO) {
        return;
    }
    
    if (!_historyFilled && _historyIndex < 2) {
        print(LOG_LEVEL_INFO, "MEM", "Pas assez de données pour afficher un graphique");
        return;
    }
    
    uint32_t maxValue = 0;
    uint32_t minValue = UINT32_MAX;
    int count = _historyFilled ? MEM_HISTORY_SIZE : _historyIndex;
    
    for (int i = 0; i < count; i++) {
        if (_freeHeapHistory[i] > maxValue) maxValue = _freeHeapHistory[i];
        if (_minFreeHeapHistory[i] < minValue && _minFreeHeapHistory[i] > 0) minValue = _minFreeHeapHistory[i];
    }
    
    maxValue = (maxValue * 105) / 100;
    minValue = minValue > 1000 ? (minValue * 95) / 100 : 0;
    
    Serial.println();
    Serial.println("\033[36m======= GRAPHIQUE D'UTILISATION DE LA MÉMOIRE =======\033[0m");
    Serial.printf("Plage: %u - %u octets | Période: %d échantillons\n", minValue, maxValue, count);
    Serial.println("\033[32m┌─────────────────────────────────────────────┐\033[0m");
    
    const int graphHeight = 10;
    
    for (int row = 0; row < graphHeight; row++) {
        Serial.print("\033[32m│\033[0m ");
        
        uint32_t threshold = maxValue - (row * (maxValue - minValue) / graphHeight);
        
        for (int i = 0; i < count; i++) {
            int idx = _historyFilled ? (_historyIndex + i) % MEM_HISTORY_SIZE : i;
            
            if (_freeHeapHistory[idx] >= threshold) {
                Serial.print("\033[32m█\033[0m");
            } else if (_minFreeHeapHistory[idx] >= threshold) {
                Serial.print("\033[33m▒\033[0m");
            } else {
                Serial.print(" ");
            }
        }
        
        Serial.printf(" \033[32m│\033[0m %u\n", threshold);
    }
    
    Serial.println("\033[32m└─────────────────────────────────────────────┘\033[0m");
    Serial.println("\033[32m█\033[0m Mémoire libre   \033[33m▒\033[0m Mémoire utilisée temporairement");
    Serial.println();
#endif
}

// Vérification de l'initialisation
bool LoggingModule::isInitialized() const {
    return _initialized;
}

// ===== FONCTIONS GLOBALES POUR LA COMPATIBILITÉ =====

// Initialisation du système de journalisation
void logInit(LogLevel level, unsigned long baudRate) {
    LoggingModule::getInstance()->init(level, baudRate);
}

// Définition du niveau de journalisation
void logSetLevel(LogLevel level) {
    LoggingModule::getInstance()->setLevel(level);
}

// Affichage d'un message de journalisation
void logPrint(LogLevel level, const char* tag, const char* format, ...) {
    if (level > currentLogLevel || level == LOG_LEVEL_NONE) {
        return;
    }
    
    LoggingModule* logger = LoggingModule::getInstance();
    if (!logger || !logger->isInitialized()) {
        return;
    }

    va_list args;
    va_start(args, format);
    char tempFormatBuffer[LOG_BUFFER_SIZE];
    vsnprintf(tempFormatBuffer, sizeof(tempFormatBuffer), format, args);
    va_end(args);
    
    logger->print(level, tag, "%s", tempFormatBuffer); 
}

// Conversion d'un niveau de journalisation en chaîne
const char* logLevelToString(LogLevel level) {
    return LoggingModule::getInstance()->levelToString(level);
}

// Journalisation de l'utilisation de la mémoire
void logMemoryUsage(const char* tag) {
    LoggingModule::getInstance()->logMemoryUsage(tag);
}

// Affichage d'un graphique de l'utilisation de la mémoire
void logMemoryGraph() {
    LoggingModule::getInstance()->logMemoryGraph();
}
