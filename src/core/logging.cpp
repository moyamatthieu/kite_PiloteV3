/*
  -----------------------
  Kite PiloteV3 - Module de journalisation (Implémentation)
  -----------------------
  
  Implémentation du système de journalisation avancé, optimisé pour les performances.
  
  Version: 2.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "core/logging.h"
#include <stdarg.h>

// Variable globale pour le niveau de journalisation actuel
uint8_t currentLogLevel = LOG_LEVEL_INFO; // Niveau par défaut

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
#else
static const char* LOG_COLORS[] = {
  "",           // NONE - pas de couleur
  "\033[31m",   // ERROR - rouge
  "\033[33m",   // WARNING - jaune
  "\033[32m",   // INFO - vert
  "\033[36m"    // DEBUG - cyan
};
#endif

#if MEMORY_OPTIMIZATION_ENABLED
static const char* colorReset = "";
#else
static const char* colorReset = "\033[0m"; // Renommé pour éviter les conflits
#endif

// Timestamp de démarrage pour calculer le temps relatif
static unsigned long startTime = 0;

// Buffer statique pour les messages du journal
static char logBuffer[LOG_BUFFER_SIZE];

/**
 * Initialise le système de journalisation.
 * @param level Niveau de log.
 * @param baudRate Vitesse de communication série.
 */
void logInit(uint8_t level, unsigned long baudRate) {
  Serial.begin(baudRate);
  while (!Serial) {
    ;
  }
  Serial.printf(COLOR_GREEN "[INFO] [LOGGING] Initialisation du système de log au niveau %d" colorReset "\n", level);
}

/**
 * Définit le niveau de journalisation
 * @param level Niveau de log.
 */
void logSetLevel(uint8_t level) {
  currentLogLevel = level;
  logPrint(LOG_LEVEL_INFO, "LOG", "Niveau de journalisation changé: %s", logLevelToString(level));
}

/**
 * Affiche un message de journalisation
 * @param level Niveau de log.
 * @param tag Tag du message.
 * @param format Format du message.
 * @param ... Arguments variables.
 */
void logPrint(uint8_t level, const char* tag, const char* format, ...) {
  if (level > currentLogLevel || level == LOG_LEVEL_NONE) {
    return;
  }
  
#if MEMORY_OPTIMIZATION_ENABLED
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;
  size_t headerLength = snprintf(logBuffer, LOG_BUFFER_SIZE, 
                 "[%lu.%03u] %s %s: ",
                 elapsedTime / 1000, elapsedTime % 1000,
                 LOG_LEVEL_NAMES[level], tag);
  
  if (headerLength >= LOG_BUFFER_SIZE - 1) {
    Serial.println("BUFFER OVERFLOW");
    return;
  }
  
  va_list args;
  va_start(args, format);
  vsnprintf(logBuffer + headerLength, LOG_BUFFER_SIZE - headerLength, format, args);
  va_end(args);
  
  Serial.println(logBuffer);
  
#else
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;
  unsigned long seconds = elapsedTime / 1000;
  unsigned int milliseconds = elapsedTime % 1000;
  
  size_t headerLength = snprintf(logBuffer, LOG_BUFFER_SIZE, 
                     "%s[%6lu.%03u] %7s %-10s%s: ",
                     LOG_COLORS[level],
                     seconds, milliseconds,
                     LOG_LEVEL_NAMES[level],
                     tag,
                     colorReset);
  
  if (headerLength >= LOG_BUFFER_SIZE - 1) {
    Serial.println("LOG BUFFER OVERFLOW DANS L'EN-TÊTE");
    return;
  }
  
  va_list args;
  va_start(args, format);
  size_t messageLength = vsnprintf(logBuffer + headerLength, 
                                  LOG_BUFFER_SIZE - headerLength - 1, 
                                  format, args);
  va_end(args);
  
  if (messageLength >= LOG_BUFFER_SIZE - headerLength - 1) {
    const char truncated[] = " [...]";
    strncpy(logBuffer + LOG_BUFFER_SIZE - strlen(truncated) - 1, 
            truncated, 
            strlen(truncated));
  }
  
  Serial.println(logBuffer);
#endif
}

/**
 * Obtient une représentation texte d'un niveau de journalisation
 * @param level Niveau de log.
 * @return Chaîne de caractères décrivant le niveau de log.
 */
const char* logLevelToString(uint8_t level) {
  if (level >= LOG_LEVEL_NONE && level <= LOG_LEVEL_DEBUG) {
    return LOG_LEVEL_NAMES[level];
  }
  return "UNKNOWN";
}

// Historique des valeurs de mémoire libre pour le graphique
#if MEMORY_OPTIMIZATION_ENABLED
#define MEM_HISTORY_SIZE 10
#else
#define MEM_HISTORY_SIZE 30
#endif

static uint32_t freeHeapHistory[MEM_HISTORY_SIZE] = {0};
static uint32_t minFreeHeapHistory[MEM_HISTORY_SIZE] = {0};
static int historyIndex = 0;
static bool historyFilled = false;

/**
 * Journalise l'utilisation de la mémoire
 * @param tag Tag du message.
 */
void logMemoryUsage(const char* tag) {
  if (currentLogLevel < LOG_LEVEL_INFO) {
    return;
  }
  
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t totalHeap = ESP.getHeapSize();
  uint32_t minFreeHeap = ESP.getMinFreeHeap();
  uint32_t maxAllocHeap = ESP.getMaxAllocHeap();
  
  static uint8_t skipCounter = 0;
  if (skipCounter++ % 3 == 0) {
    freeHeapHistory[historyIndex] = freeHeap;
    minFreeHeapHistory[historyIndex] = minFreeHeap;
    historyIndex = (historyIndex + 1) % MEM_HISTORY_SIZE;
    if (historyIndex == 0) {
      historyFilled = true;
    }
  }
  
#if MEMORY_OPTIMIZATION_ENABLED
  logPrint(LOG_LEVEL_INFO, tag, "Mem: %u/%u (%u%%), Min %u, Max %u", 
           freeHeap, totalHeap, (freeHeap * 100) / totalHeap, 
           minFreeHeap, maxAllocHeap);
#else
  logPrint(LOG_LEVEL_INFO, tag, "Mémoire: Libre %u/%u octets (%u%%), Min libre %u, Max bloc %u", 
           freeHeap, totalHeap, (freeHeap * 100) / totalHeap, 
           minFreeHeap, maxAllocHeap);
#endif
}

/**
 * Affiche un graphique de l'utilisation de la mémoire au fil du temps
 */
void logMemoryGraph() {
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
  
  if (!historyFilled && historyIndex < 2) {
    logPrint(LOG_LEVEL_INFO, "MEM", "Pas assez de données pour afficher un graphique");
    return;
  }
  
  uint32_t maxValue = 0;
  uint32_t minValue = UINT32_MAX;
  int count = historyFilled ? MEM_HISTORY_SIZE : historyIndex;
  
  for (int i = 0; i < count; i++) {
    if (freeHeapHistory[i] > maxValue) maxValue = freeHeapHistory[i];
    if (minFreeHeapHistory[i] < minValue && minFreeHeapHistory[i] > 0) minValue = minFreeHeapHistory[i];
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
      int idx = historyFilled ? (historyIndex + i) % MEM_HISTORY_SIZE : i;
      
      if (freeHeapHistory[idx] >= threshold) {
        Serial.print("\033[32m█\033[0m");
      } else if (minFreeHeapHistory[idx] >= threshold) {
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
