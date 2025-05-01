/*
  -----------------------
  Kite PiloteV3 - Module de journalisation
  -----------------------
  
  Système de journalisation avancé pour le suivi des erreurs et le débogage.
  
  Version: 1.0.0
  Date: 1 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef LOGGING_H
#define LOGGING_H

#include <Arduino.h>

// Niveaux de journalisation
typedef enum {
  LOG_NONE = 0,       // Pas de journalisation
  LOG_ERROR = 1,      // Erreurs critiques uniquement
  LOG_WARNING = 2,    // Erreurs et avertissements
  LOG_INFO = 3,       // Informations générales
  LOG_DEBUG = 4       // Messages de débogage détaillés
} LogLevel;

// Niveau de journalisation global
// Peut être modifié à l'exécution
extern LogLevel currentLogLevel;

// Macros pour faciliter la journalisation
#define LOG_ERROR(tag, format, ...) logPrint(LOG_ERROR, tag, format, ##__VA_ARGS__)
#define LOG_WARNING(tag, format, ...) logPrint(LOG_WARNING, tag, format, ##__VA_ARGS__)
#define LOG_INFO(tag, format, ...) logPrint(LOG_INFO, tag, format, ##__VA_ARGS__)
#define LOG_DEBUG(tag, format, ...) logPrint(LOG_DEBUG, tag, format, ##__VA_ARGS__)

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

#endif // LOGGING_H
