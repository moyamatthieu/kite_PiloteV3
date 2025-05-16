/*
  -----------------------
  Kite PiloteV3 - Système (Interface)
  -----------------------
  
  Interface pour les fonctions et constantes système de base.
  Ce fichier est un pont entre l'ancienne et la nouvelle architecture.
  
  Date: 15 mai 2025
  Auteurs: Équipe Kite PiloteV3
*/

#ifndef SYSTEM_H
#define SYSTEM_H

#include "core/component.h"
#include "core/system_orchestrator.h"
#include "common/global_enums.h"

// Codes d'erreur système (pour la compatibilité avec l'ancien code)
#define SYS_OK                   0
#define SYS_ERROR               -1
#define SYS_ERROR_INIT_FAILED   -2
#define SYS_ERROR_OUT_OF_MEMORY -3
#define SYS_ERROR_TIMEOUT       -4

// Fonction de compatibilité entre l'ancien et le nouveau système
inline int systemInit() {
    // Utilise le SystemOrchestrator de la nouvelle architecture
    SystemOrchestrator* orchestrator = SystemOrchestrator::getInstance();
    if (!orchestrator) {
        return SYS_ERROR_INIT_FAILED;
    }
    
    // Initialise le SystemOrchestrator
    if (orchestrator->initialize() != ErrorCode::OK) {
        return SYS_ERROR_INIT_FAILED;
    }
    
    return SYS_OK;
}

#endif // SYSTEM_H