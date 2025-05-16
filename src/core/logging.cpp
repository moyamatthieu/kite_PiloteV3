/*
  -----------------------
  Kite PiloteV3 - Module de journalisation (Implémentation)
  -----------------------
  
  Implémentation du système de journalisation avancé, optimisé pour les performances.
  
  Version: 2.1.0
  Date: 15 Mai 2024
  Auteurs: Équipe Kite PiloteV3
*/

#include "core/logging.h" // Utilise LogLevel de global_enums.h
#include "core/config.h"
#include <stdarg.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>  // Pour accéder à pcTaskGetName

// Définition de l'instance singleton
LoggingModule* LoggingModule::_instance = nullptr;

// Variable globale pour le niveau de journalisation actuel
LogLevel currentLogLevel = LogLevel::INFO; // Niveau par défaut, utilise l'enum class

// Tailles des buffers
#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE 192
#endif

#ifndef MEM_HISTORY_SIZE
#define MEM_HISTORY_SIZE 30
#endif

// Noms des niveaux de journalisation pour l'affichage (doit correspondre à l'ordre de LogLevel)
static const char* LOG_LEVEL_NAMES[] = {
  "NONE",    // LogLevel::NONE
  "ERROR",   // LogLevel::ERROR
  "WARNING", // LogLevel::WARNING
  "INFO",    // LogLevel::INFO
  "DEBUG"    // LogLevel::DEBUG
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
static const char* logColorReset = "\033[0m";
#endif

// Constructeur privé
LoggingModule::LoggingModule() : 
    _initialized(false),
    _startTime(0),
    _logMessageId(0),
    _historyIndex(0),
    _historyFilled(false) {
    
    _logMutex = xSemaphoreCreateMutex();
    // ... (gestion erreur mutex) ...

    _logBuffer = new char[LOG_BUFFER_SIZE];
    _freeHeapHistory = new uint32_t[MEM_HISTORY_SIZE];
    _minFreeHeapHistory = new uint32_t[MEM_HISTORY_SIZE];
    
    for (int i = 0; i < MEM_HISTORY_SIZE; i++) {
        _freeHeapHistory[i] = 0;
        _minFreeHeapHistory[i] = 0;
    }
    
    _levelNames = LOG_LEVEL_NAMES;
    _colors = LOG_COLORS;
    _colorReset = logColorReset;
}

// Destructeur
LoggingModule::~LoggingModule() {
    if (_logBuffer) delete[] _logBuffer;
    if (_freeHeapHistory) delete[] _freeHeapHistory;
    if (_minFreeHeapHistory) delete[] _minFreeHeapHistory;
    if (_logMutex != NULL) vSemaphoreDelete(_logMutex);
}

// Obtention de l'instance unique
LoggingModule* LoggingModule::getInstance() {
    if (_instance == nullptr) {
        _instance = new LoggingModule();
    }
    return _instance;
}

// Initialisation du système de journalisation
bool LoggingModule::init(LogLevel level, unsigned long baudRate) { // Signature utilise LogLevel
    if (_initialized) {
        return true;
    }
    
    if (!Serial) {
        Serial.begin(baudRate);
        // ... (attente initialisation Serial) ...
    }
    
    _startTime = millis();
    currentLogLevel = level; // Utilise LogLevel
    _initialized = true;
    
    Serial.printf("\033[32m[INFO] [LOGGING] Initialisation du système de log au niveau %s\033[0m\n", levelToString(level));
    
    return true;
}

// Définition du niveau de journalisation
void LoggingModule::setLevel(LogLevel level) { // Signature utilise LogLevel
    currentLogLevel = level; // Utilise LogLevel
    Serial.printf("\033[32m[INFO] [LOG] Niveau de journalisation changé: %s\033[0m\n", levelToString(level));
}

// Obtention du niveau de journalisation actuel
LogLevel LoggingModule::getLevel() const { // Type de retour LogLevel
    return currentLogLevel;
}

// Conversion d'un niveau de journalisation en chaîne
const char* LoggingModule::levelToString(LogLevel level) const { // Signature utilise LogLevel
    uint8_t level_idx = static_cast<uint8_t>(level);
    if (level_idx < (sizeof(LOG_LEVEL_NAMES) / sizeof(LOG_LEVEL_NAMES[0]))) {
        return _levelNames[level_idx];
    }
    return "UNKNOWN";
}

// Affichage d'un message de journalisation
void LoggingModule::print(LogLevel level, const char* tag, const char* format, ...) { // Signature utilise LogLevel
    if (level > currentLogLevel || level == LogLevel::NONE || !_initialized) { // Compare avec LogLevel
        return;
    }

    if (_logMutex != NULL) {
        if (xSemaphoreTake(_logMutex, portMAX_DELAY) == pdTRUE) {
            va_list args;
            va_start(args, format);
            vprint(level, tag, format, args); // level est de type LogLevel
            va_end(args);
            xSemaphoreGive(_logMutex);
        }
    } else {
        va_list args;
        va_start(args, format);
        vprint(level, tag, format, args); // level est de type LogLevel
        va_end(args);
    }
}

// Méthode pour le formatage et l'impression
void LoggingModule::vprint(LogLevel level, const char* tag, const char* format, va_list args) { // Signature utilise LogLevel
    _logMessageId++; 

    char taskName[configMAX_TASK_NAME_LEN] = "main";
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();
        if (currentTask != NULL) {
            const char* name = pcTaskGetName(currentTask);
            if (name != NULL) {
                strncpy(taskName, name, sizeof(taskName) - 1);
                taskName[sizeof(taskName) - 1] = '\0';
            }
        }
    }
    
    uint8_t level_idx = static_cast<uint8_t>(level);
    const char* color = (level_idx < (sizeof(_colors)/sizeof(_colors[0]))) ? _colors[level_idx] : "";
    const char* levelName = levelToString(level);

#if MEMORY_OPTIMIZATION_ENABLED
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - _startTime;
    size_t headerLength = snprintf(_logBuffer, LOG_BUFFER_SIZE, 
                   "[ID:%lu][%lu.%03u] %s %s [%s]: ",
                   _logMessageId,
                   elapsedTime / 1000, elapsedTime % 1000,
                   levelName, tag, taskName);
    
    if (headerLength >= LOG_BUFFER_SIZE - 1) {
        Serial.println("[LOG_ERR] Buffer overflow in header (optimized)");
        return;
    }
    
    vsnprintf(_logBuffer + headerLength, LOG_BUFFER_SIZE - headerLength, format, args);
    Serial.println(_logBuffer);
    
#else // Mode non optimisé (avec couleurs)
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - _startTime;
    unsigned long seconds = elapsedTime / 1000;
    unsigned int milliseconds = elapsedTime % 1000;
    
    size_t headerLength = snprintf(_logBuffer, LOG_BUFFER_SIZE, 
                       "%s[ID:%lu][%6lu.%03u] %7s %-10s [%-*s]%s: ",
                       color,
                       _logMessageId, 
                       seconds, milliseconds,
                       levelName,
                       tag,
                       configMAX_TASK_NAME_LEN -1,
                       taskName,
                       _colorReset);
    
    if (headerLength >= LOG_BUFFER_SIZE - 1) {
        Serial.printf("%s[LOG_ERR] Buffer overflow in header (colored)%s\n", _colors[static_cast<uint8_t>(LogLevel::ERROR)], _colorReset);
        return;
    }
    
    size_t remainingBuffer = LOG_BUFFER_SIZE - headerLength;
    if (remainingBuffer > 1) {
        vsnprintf(_logBuffer + headerLength, remainingBuffer, format, args);
    }
    
    Serial.println(_logBuffer);
#endif
}

// Journalisation de l'utilisation de la mémoire
void LoggingModule::logMemoryUsage(const char* tag) {
    if (currentLogLevel < LogLevel::INFO) { // Compare avec LogLevel
        return;
    }
    // ... (corps de la fonction inchangé, mais print interne utilisera LogLevel::INFO) ...
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
    print(LogLevel::INFO, tag, "Mem: %u/%u (%u%%), Min %u, Max %u",
             freeHeap, totalHeap, (freeHeap * 100) / totalHeap, 
             minFreeHeap, maxAllocHeap);
#else
    print(LogLevel::INFO, tag, "Mémoire: Libre %u/%u octets (%u%%), Min libre %u, Max bloc %u",
             freeHeap, totalHeap, (freeHeap * 100) / totalHeap, 
             minFreeHeap, maxAllocHeap);
#endif
}

// Affichage d'un graphique de l'utilisation de la mémoire
void LoggingModule::logMemoryGraph() {
#if MEMORY_OPTIMIZATION_ENABLED
  if (currentLogLevel < LogLevel::INFO) { // Compare avec LogLevel
        return;
    }
    // ... (corps inchangé) ...
#else
    if (currentLogLevel < LogLevel::INFO) { // Compare avec LogLevel
        return;
    }
    
    if (!_historyFilled && _historyIndex < 2) {
        print(LogLevel::INFO, "MEM", "Pas assez de données pour afficher un graphique"); // Utilise LogLevel::INFO
        return;
    }
    // ... (corps inchangé) ...
#endif
}

// Vérification de l'initialisation
bool LoggingModule::isInitialized() const {
    return _initialized;
}

// ===== FONCTIONS GLOBALES POUR LA COMPATIBILITÉ =====

// Initialisation du système de journalisation
void logInit(LogLevel level, unsigned long baudRate) { // Signature utilise LogLevel
    LoggingModule::getInstance()->init(level, baudRate);
}

// Définition du niveau de journalisation
void logSetLevel(LogLevel level) { // Signature utilise LogLevel
    LoggingModule::getInstance()->setLevel(level);
}

// La fonction globale logPrint(LogLevel level, const char* tag, const char* format, ...) est supprimée
// car les macros LOG_INFO, etc. appellent directement LoggingModule::getInstance()->print.

// Conversion d'un niveau de journalisation en chaîne
const char* logLevelToString(LogLevel level) { // Signature utilise LogLevel
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
