#pragma once

#include "hal/hal_component.h"
#include "common/global_enums.h"
#include <Arduino.h> // Pour millis()

// Structure pour les données traitées du capteur de vent
struct ProcessedWindData {
    float speed;         // Vitesse du vent en m/s
    float direction;     // Direction du vent en degrés (0-359)
    float gust;          // Rafale maximale en m/s (sur une période donnée)
    uint32_t timestamp;  // Horodatage de la mesure
    bool isValid;        // Indique si les données sont valides

    ProcessedWindData() :
        speed(0.0f), direction(0.0f), gust(0.0f), timestamp(0), isValid(false) {}
};

// Structure pour la configuration du capteur de vent
struct WindDriverConfig {
    // Exemple: broche pour un anémomètre à impulsions, adresse I2C pour un capteur numérique, etc.
    uint8_t anemometerPin; // Si applicable
    uint8_t windVanePin;   // Si applicable (potentiomètre ou encodeur)
    // Autres paramètres spécifiques au matériel
    float calibrationFactorSpeed;    // Facteur de calibration pour la vitesse
    float calibrationOffsetDirection; // Offset de calibration pour la direction

    WindDriverConfig() :
        anemometerPin(0xFF), // Valeur invalide par défaut, à configurer
        windVanePin(0xFF),   // Valeur invalide par défaut, à configurer
        calibrationFactorSpeed(1.0f),
        calibrationOffsetDirection(0.0f) {}
};

class WindDriver : public SensorComponent {
public:
    WindDriver(const char* name = "WindDriver");
    ~WindDriver() override = default;

    ErrorCode initialize() override;
    bool read() override; // Implémentation de la méthode virtuelle pure
    ErrorCode configure(const WindDriverConfig& config);

    // Méthodes spécifiques (si nécessaire)
    ErrorCode calibrateSpeed(float knownWindSpeed); // Exemple de calibration
    ErrorCode calibrateDirection(float knownWindDirection); // Exemple de calibration
    bool selfTest(); // Si un auto-test matériel est possible

    const ProcessedWindData& getProcessedData() const { return _processedData; }

    // Méthodes d'accès aux données spécifiques
    bool dataAvailable() const;
    float getWindSpeed() const;
    float getWindDirection() const;

protected:
    void onEnable() override;
    void onDisable() override;

private:
    ErrorCode applyConfig();
    void readSensor();
    void processSensorData(); // Traite les données brutes en ProcessedWindData

    WindDriverConfig _config;
    ProcessedWindData _processedData;
    // Variables internes pour le calcul des rafales, etc.
    float _maxGustSinceLastRead;
    uint32_t _lastGustResetTime;
};
