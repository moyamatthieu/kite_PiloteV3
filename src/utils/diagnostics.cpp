#include "../../include/utils/diagnostics.h"
#include "../../include/utils/logging.h"
#include "../../include/hardware/io/display_manager.h"
#include "../../include/core/system.h"
#include "utils/diagnostics.h"
#include "core/logging.h" // Pour LOG_INFO, LOG_ERROR
#include <Wire.h>          // Pour la communication I2C

// Implémentation de la fonction de scan du bus I2C
void scanI2CBus() {
  LOG_INFO("I2C_SCAN", "Scanning I2C bus...");
  byte error, address;
  int nDevices;

  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      LOG_INFO("I2C_SCAN", "I2C device found at address 0x%02X", address);
      nDevices++;
    } else if (error==4) {
      // Ne pas logger une erreur ici, car c'est normal de ne pas trouver de périphérique lors d'un scan
    }    
  }
  if (nDevices == 0) {
    LOG_INFO("I2C_SCAN", "No I2C devices found on the bus.");
  } else {
    LOG_INFO("I2C_SCAN", "Scan complete. Found %d device(s).", nDevices);
  }
}

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