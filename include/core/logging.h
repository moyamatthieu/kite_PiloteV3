/*
  -----------------------
  Kite PiloteV3 - Module de journalisation (Interface)
  -----------------------
  
  Système de journalisation avancé avec plusieurs niveaux, codes couleur et formatage.
  
  Version: 2.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef LOGGING_H
#define LOGGING_H

#include <Arduino.h>
#include "config.h"

// Type pour les niveaux de journalisation (utilise les valeurs définies dans config.h)
typedef enum {
  LOG_NONE = LOG_LEVEL_NONE,
  LOG_ERROR = LOG_LEVEL_ERROR,
  LOG_WARNING = LOG_LEVEL_WARNING,
  LOG_INFO = LOG_LEVEL_INFO,
  LOG_DEBUG = LOG_LEVEL_DEBUG
} LogLevel;

// Macros de journalisation
#define LOG_ERROR(tag, format, ...)   logPrint((LogLevel)LOG_LEVEL_ERROR, tag, format, ##__VA_ARGS__)
#define LOG_WARNING(tag, format, ...) logPrint((LogLevel)LOG_LEVEL_WARNING, tag, format, ##__VA_ARGS__)
#define LOG_INFO(tag, format, ...)    logPrint((LogLevel)LOG_LEVEL_INFO, tag, format, ##__VA_ARGS__)
#define LOG_DEBUG(tag, format, ...)   logPrint((LogLevel)LOG_LEVEL_DEBUG, tag, format, ##__VA_ARGS__)

// Fonctions principales
void logInit(LogLevel level, unsigned long baudRate);
void logPrint(LogLevel level, const char* tag, const char* format, ...);
void logSetLevel(LogLevel level);
void logMemoryUsage(const char* tag);
void logMemoryGraph();

// Déclaration externe de la variable de niveau de journalisation
extern LogLevel currentLogLevel;

// Déclaration externe des noms des niveaux de journalisation
extern const char* logLevelToString(LogLevel level);

#endif // LOGGING_H
