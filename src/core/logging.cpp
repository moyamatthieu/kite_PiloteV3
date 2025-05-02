/*
  -----------------------
  Kite PiloteV3 - Module de journalisation (Implémentation)
  -----------------------
  
  Implémentation du système de journalisation avancé, optimisé pour les performances.
  
  Version: 2.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#include "../include/logging.h"
#include <stdarg.h>
#include "../include/config.h"

// Variable globale pour le niveau de journalisation actuel
LogLevel currentLogLevel = LOG_INFO; // Niveau par défaut

// Noms des niveaux de journalisation pour l'affichage
static const char* LOG_LEVEL_NAMES[] = {
  "NONE",
  "ERROR",
  "WARNING",
  "INFO",
  "DEBUG"
};

// Couleurs ANSI pour le terminal (pour une meilleure lisibilité)
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
static const char* COLOR_RESET = "";
#else
static const char* COLOR_RESET = "\033[0m";
#endif

// Timestamp de démarrage pour calculer le temps relatif
static unsigned long startTime = 0;

// Buffer statique pour les messages du journal
static char logBuffer[LOG_BUFFER_SIZE];

/**
 * Initialise le système de journalisation
 */
void logInit(LogLevel level, unsigned long baudRate) {
  Serial.begin(baudRate);
  // Petit délai pour s'assurer que le port série est prêt
  delay(50); // Réduit de 100 à 50ms
  
  currentLogLevel = level;
  startTime = millis();
  
  // Message initial
  logPrint(LOG_INFO, "LOG", "Système de journalisation initialisé (niveau: %s)", logLevelToString(level));
}

/**
 * Définit le niveau de journalisation
 */
void logSetLevel(LogLevel level) {
  currentLogLevel = level;
  logPrint(LOG_INFO, "LOG", "Niveau de journalisation changé: %s", logLevelToString(level));
}

/**
 * Affiche un message de journalisation - version optimisée
 */
void logPrint(LogLevel level, const char* tag, const char* format, ...) {
  // Ne rien faire si le niveau est trop bas
  if (level > currentLogLevel || level == LOG_NONE) {
    return;
  }
  
#if MEMORY_OPTIMIZATION_ENABLED
  // Version optimisée pour la production avec moins de formatage
  
  // Formater le temps depuis le démarrage de façon concise
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;
  
  // En-tête du message avec timestamp et niveau
  size_t headerLength = snprintf(logBuffer, LOG_BUFFER_SIZE, 
                 "[%lu.%03u] %s %s: ",
                 elapsedTime / 1000, elapsedTime % 1000,
                 LOG_LEVEL_NAMES[level], tag);
  
  // Vérifier si le buffer est déjà plein après l'en-tête
  if (headerLength >= LOG_BUFFER_SIZE - 1) {
    Serial.println("BUFFER OVERFLOW");
    return;
  }
  
  // Formater le message sans allocations supplémentaires
  va_list args;
  va_start(args, format);
  vsnprintf(logBuffer + headerLength, LOG_BUFFER_SIZE - headerLength, format, args);
  va_end(args);
  
  // Afficher directement pour éviter les manipulations de chaînes supplémentaires
  Serial.println(logBuffer);
  
#else
  // Version complète avec coloration pour le développement
  
  // Formater le temps depuis le démarrage
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;
  unsigned long seconds = elapsedTime / 1000;
  unsigned int milliseconds = elapsedTime % 1000;
  
  // En-tête du message avec timestamp et niveau
  size_t headerLength = snprintf(logBuffer, LOG_BUFFER_SIZE, 
                     "%s[%6lu.%03u] %7s %-10s%s: ",
                     LOG_COLORS[level],
                     seconds, milliseconds,
                     LOG_LEVEL_NAMES[level],
                     tag,
                     COLOR_RESET);
  
  // S'assurer que l'en-tête est correctement formaté et qu'il reste de la place
  if (headerLength >= LOG_BUFFER_SIZE - 1) {
    Serial.println("LOG BUFFER OVERFLOW DANS L'EN-TÊTE");
    return;
  }
  
  // Formater le message avec les arguments variables
  va_list args;
  va_start(args, format);
  size_t messageLength = vsnprintf(logBuffer + headerLength, 
                                  LOG_BUFFER_SIZE - headerLength - 1, 
                                  format, args);
  va_end(args);
  
  // Vérifier si le message a pu être correctement formaté
  if (messageLength >= LOG_BUFFER_SIZE - headerLength - 1) {
    // Ajouter une indication que le message a été tronqué
    const char truncated[] = " [...]";
    strncpy(logBuffer + LOG_BUFFER_SIZE - strlen(truncated) - 1, 
            truncated, 
            strlen(truncated));
  }
  
  // Afficher le message sur la console série
  Serial.println(logBuffer);
#endif
}

/**
 * Obtient une représentation texte d'un niveau de journalisation
 */
const char* logLevelToString(LogLevel level) {
  if (level >= LOG_NONE && level <= LOG_DEBUG) {
    return LOG_LEVEL_NAMES[level];
  }
  return "UNKNOWN";
}

// Historique des valeurs de mémoire libre pour le graphique - version optimisée
#if MEMORY_OPTIMIZATION_ENABLED
#define MEM_HISTORY_SIZE 10  // Réduit à 10 au lieu de 30
#else
#define MEM_HISTORY_SIZE 30
#endif

static uint32_t freeHeapHistory[MEM_HISTORY_SIZE] = {0};
static uint32_t minFreeHeapHistory[MEM_HISTORY_SIZE] = {0};
static int historyIndex = 0;
static bool historyFilled = false;

/**
 * Journalise l'utilisation de la mémoire - version optimisée
 */
void logMemoryUsage(const char* tag) {
  if (currentLogLevel < LOG_INFO) {
    return;
  }
  
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t totalHeap = ESP.getHeapSize();
  uint32_t minFreeHeap = ESP.getMinFreeHeap();
  uint32_t maxAllocHeap = ESP.getMaxAllocHeap();
  
  // Enregistrer dans l'historique (tous les 3 appels seulement)
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
  // Version concise pour la production
  logPrint(LOG_INFO, tag, "Mem: %u/%u (%u%%), Min %u, Max %u", 
           freeHeap, totalHeap, (freeHeap * 100) / totalHeap, 
           minFreeHeap, maxAllocHeap);
#else
  // Version détaillée pour le développement
  logPrint(LOG_INFO, tag, "Mémoire: Libre %u/%u octets (%u%%), Min libre %u, Max bloc %u", 
           freeHeap, totalHeap, (freeHeap * 100) / totalHeap, 
           minFreeHeap, maxAllocHeap);
#endif
}

/**
 * Affiche un graphique de l'utilisation de la mémoire au fil du temps - version optimisée
 */
void logMemoryGraph() {
#if MEMORY_OPTIMIZATION_ENABLED
  // En mode production, afficher uniquement un résumé compact
  if (currentLogLevel < LOG_INFO) {
    return;
  }
  
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t minFreeHeap = ESP.getMinFreeHeap();
  uint32_t maxUsed = ESP.getHeapSize() - minFreeHeap;
  
  Serial.println("--- MÉMOIRE ---");
  Serial.printf("Libre: %u  |  Min libre: %u  |  Max utilisé: %u\n", 
                freeHeap, minFreeHeap, maxUsed);
#else
  // Version complète avec graphique pour le développement
  if (currentLogLevel < LOG_INFO) {
    return;
  }
  
  if (!historyFilled && historyIndex < 2) {
    logPrint(LOG_INFO, "MEM", "Pas assez de données pour afficher un graphique");
    return;
  }
  
  // Déterminer les valeurs min/max pour l'échelle
  uint32_t maxValue = 0;
  uint32_t minValue = UINT32_MAX;
  int count = historyFilled ? MEM_HISTORY_SIZE : historyIndex;
  
  for (int i = 0; i < count; i++) {
    if (freeHeapHistory[i] > maxValue) maxValue = freeHeapHistory[i];
    if (minFreeHeapHistory[i] < minValue && minFreeHeapHistory[i] > 0) minValue = minFreeHeapHistory[i];
  }
  
  // Ajouter une marge
  maxValue = (maxValue * 105) / 100;
  minValue = minValue > 1000 ? (minValue * 95) / 100 : 0;
  
  // En-tête du graphique
  Serial.println();
  Serial.println("\033[36m======= GRAPHIQUE D'UTILISATION DE LA MÉMOIRE =======\033[0m");
  Serial.printf("Plage: %u - %u octets | Période: %d échantillons\n", minValue, maxValue, count);
  Serial.println("\033[32m┌─────────────────────────────────────────────┐\033[0m");
  
  // Hauteur du graphique
  const int graphHeight = 10; // Réduit de 15 à 10
  
  // Dessiner le graphique ligne par ligne
  for (int row = 0; row < graphHeight; row++) {
    Serial.print("\033[32m│\033[0m ");
    
    uint32_t threshold = maxValue - (row * (maxValue - minValue) / graphHeight);
    
    for (int i = 0; i < count; i++) {
      int idx = historyFilled ? (historyIndex + i) % MEM_HISTORY_SIZE : i;
      
      if (freeHeapHistory[idx] >= threshold) {
        Serial.print("\033[32m█\033[0m"); // Mémoire libre
      } else if (minFreeHeapHistory[idx] >= threshold) {
        Serial.print("\033[33m▒\033[0m"); // Zone entre libre et min libre
      } else {
        Serial.print(" ");
      }
    }
    
    // Afficher l'échelle à droite
    Serial.printf(" \033[32m│\033[0m %u\n", threshold);
  }
  
  // Pied du graphique
  Serial.println("\033[32m└─────────────────────────────────────────────┘\033[0m");
  Serial.println("\033[32m█\033[0m Mémoire libre   \033[33m▒\033[0m Mémoire utilisée temporairement");
  Serial.println();
#endif
}
