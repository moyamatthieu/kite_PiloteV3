/*
  -----------------------
  Kite PiloteV3 - Module de journalisation
  -----------------------
  
  Système de journalisation avancé pour le suivi des erreurs et le débogage.
  
  Version: 1.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

/* === MODULE LOGGING ===
   Ce module fournit un système de journalisation avancé avec différents niveaux de log
   (ERROR, WARNING, INFO, DEBUG) et des options de formatage pour faciliter le débogage.
*/

#ifndef LOGGING_H
#define LOGGING_H

#include <Arduino.h>

// === NIVEAUX DE LOG ===
typedef uint8_t LogLevel;
#define LOG_LEVEL_NONE    0  // Pas de journalisation
#define LOG_LEVEL_ERROR   1  // Erreurs critiques uniquement
#define LOG_LEVEL_WARNING 2  // Erreurs et avertissements
#define LOG_LEVEL_INFO    3  // Informations générales
#define LOG_LEVEL_DEBUG   4  // Messages de débogage détaillés

// === COULEURS POUR LES LOGS ===
#define COLOR_RESET   "\033[0m"   // Réinitialise la couleur
#define COLOR_RED     "\033[31m"  // Rouge pour les erreurs
#define COLOR_YELLOW  "\033[33m"  // Jaune pour les avertissements
#define COLOR_GREEN   "\033[32m"  // Vert pour les informations
#define COLOR_CYAN    "\033[36m"  // Cyan pour le débogage

// === CONFIGURATION ===
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO  // Niveau de log par défaut
#endif

// === MACROS DE LOGGING ===
  // Log d'erreur
  #define LOG_ERROR(tag, format, ...) \
    if (LOG_LEVEL >= LOG_LEVEL_ERROR) { \
      Serial.printf(COLOR_RED "[ERROR] [%s] " format COLOR_RESET "\n", tag, ##__VA_ARGS__); \
    }

  // Log d'avertissement
  #define LOG_WARNING(tag, format, ...) \
    if (LOG_LEVEL >= LOG_LEVEL_WARNING) { \
      Serial.printf(COLOR_YELLOW "[WARNING] [%s] " format COLOR_RESET "\n", tag, ##__VA_ARGS__); \
    }

  // Log d'information
  #define LOG_INFO(tag, format, ...) \
    if (LOG_LEVEL >= LOG_LEVEL_INFO) { \
      Serial.printf(COLOR_GREEN "[INFO] [%s] " format COLOR_RESET "\n", tag, ##__VA_ARGS__); \
    }

  // Log de débogage
  #define LOG_DEBUG(tag, format, ...) \
    if (LOG_LEVEL >= LOG_LEVEL_DEBUG) { \
      Serial.printf(COLOR_CYAN "[DEBUG] [%s] " format COLOR_RESET "\n", tag, ##__VA_ARGS__); \
    }

// === FONCTIONS ===

/**
 * Initialise le système de journalisation.
 * @param level Niveau de log (LOG_NONE, LOG_ERROR, etc.).
 * @param baudRate Vitesse de communication série.
 */
void logInit(uint8_t level, unsigned long baudRate);

#endif // LOGGING_H
