#pragma once

#include "hal/hal_component.h"
#include "common/global_enums.h"
#include <Arduino.h> // Pour millis()

// Structure pour les données traitées du capteur de tension
struct ProcessedTensionData {
    float tension;          // Tension en Newtons (N)
    float rawValue;         // Valeur brute du capteur (ex: ADC)
    uint32_t timestamp;     // Horodatage de la mesure
    bool isValid;           // Indique si les données sont valides

    ProcessedTensionData() :
        tension(0.0f), rawValue(0.0f), timestamp(0), isValid(false) {}
};

// Structure pour la configuration du capteur de tension
struct TensionDriverConfig {
    uint8_t sensorPin;          // Broche analogique pour le capteur de tension (cellule de charge + amplificateur)
    float adcResolution;        // Résolution du CAN (ex: 4095 pour 12 bits)
    float adcVoltageReference;  // Tension de référence du CAN (ex: 3.3V)
    float amplifierGain;        // Gain de l'amplificateur de la cellule de charge
    float loadCellSensitivity;  // Sensibilité de la cellule de charge (ex: mV/V)
    float loadCellCapacity;     // Capacité maximale de la cellule de charge (ex: en kg ou N)
    float calibrationFactor;    // Facteur de calibration (N/mV ou N/ADC_unit)
    float zeroOffset;           // Offset pour la tare (en mV ou ADC_unit)

    TensionDriverConfig() :
        sensorPin(0xFF), // Invalide par défaut
        adcResolution(4095.0f),
        adcVoltageReference(3.3f),
        amplifierGain(128.0f), // Exemple pour un HX711
        loadCellSensitivity(1.0f), // mV/V, à spécifier pour la cellule utilisée
        loadCellCapacity(100.0f), // Exemple: 100N
        calibrationFactor(1.0f), // À déterminer par calibration
        zeroOffset(0.0f) {}
};

class TensionDriver : public SensorComponent {
public:
    TensionDriver(const char* name = "TensionDriver");
    ~TensionDriver() override = default;

    ErrorCode initialize() override;
    bool read() override; // Lit et traite les données du capteur
    ErrorCode configure(const TensionDriverConfig& config);

    // Méthodes spécifiques
    ErrorCode calibrateZero(); // Tare (offset)
    ErrorCode calibrateScale(float knownWeight); // Calibration de l'échelle avec un poids connu (en Newtons)
    bool selfTest(); // Si un auto-test est possible

    void readSensor(); // Ajouté pour correspondre à l'implémentation

    const ProcessedTensionData& getProcessedData() const { return _processedData; }

    // Méthodes d'accès aux données spécifiques
    bool dataAvailable() const;
    float getTension() const;

protected:
    void onEnable() override;
    void onDisable() override;

private:
    ErrorCode applyConfig();
    float readRawValue(); // Lit la valeur brute du CAN
    void processSensorData(float rawValue); // Traite la valeur brute

    TensionDriverConfig _config;
    ProcessedTensionData _processedData;
};
