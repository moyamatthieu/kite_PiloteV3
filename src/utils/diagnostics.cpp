#include "../../include/utils/diagnostics.h"
#include "../../include/utils/logging.h"
#include "../../include/hardware/io/display_manager.h"
#include "../../include/core/system.h"

void OTAManager::handleOTAEnd(bool success, int ledPin, DisplayManager& display) {
    // LED éteinte après la mise à jour
    digitalWrite(ledPin, LOW);
    
    display.displayOTAStatus(success);
    
    if (success) {
        LOG_INFO("OTA", "Mise à jour terminée avec succès");
        
        // Double clignotement pour indiquer le succès
        for (int i = 0; i < 2; i++) {
            digitalWrite(ledPin, HIGH);
            delay(50);
            digitalWrite(ledPin, LOW);
            delay(50);
        }
        
        // Redémarrer après une courte pause
        systemRestart(3000);
    } else {
        LOG_ERROR("OTA", "Erreur durant la mise à jour OTA");
        
        // Clignotement rapide pour indiquer l'erreur
        for (int i = 0; i < 5; i++) {
            digitalWrite(ledPin, HIGH);
            delay(25);
            digitalWrite(ledPin, LOW);
            delay(25);
        }
    }
}