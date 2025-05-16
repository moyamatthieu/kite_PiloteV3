#pragma once

#include "hal/hal_component.h" // Modifié pour inclure le bon fichier d'en-tête
#include "common/global_enums.h"
#include "core/config.h" // Pour les broches, la résolution ADC, etc.

#define MAX_POTENTIOMETERS 2 // Nombre maximum de potentiomètres gérés

// Énumération pour identifier les potentiomètres logiques
enum class PotentiometerType : uint8_t {
    TUNING_1 = 0,
    TUNING_2 = 1,
    // Ajoutez d'autres types si nécessaire
    NONE = 0xFF
};

// Structure pour la configuration d'un potentiomètre individuel
struct PotentiometerConfig {
    uint8_t pin;            // Broche ADC
    PotentiometerType logicalType;
    int rawMin;             // Valeur brute minimale attendue de l'ADC
    int rawMax;             // Valeur brute maximale attendue de l'ADC
    float outputMin;        // Valeur de sortie mappée minimale (par ex. 0.0f)
    float outputMax;        // Valeur de sortie mappée maximale (par ex. 100.0f)
    float smoothingFactor;  // Facteur de lissage (0.0 à 1.0). 0 = pas de lissage, 1 = lissage max (valeur ne change jamais)

    // État interne
    int lastRawValue;
    float smoothedValue;

    PotentiometerConfig() :
        pin(0xFF), logicalType(PotentiometerType::NONE),
        rawMin(POT_DEFAULT_ADC_MIN), rawMax(POT_DEFAULT_ADC_MAX),
        outputMin(0.0f), outputMax(100.0f), smoothingFactor(POT_DEFAULT_SMOOTHING_FACTOR),
        lastRawValue(0), smoothedValue(0.0f) {}
};

class PotentiometerDriver : public InputComponent { // Hérite de InputComponent qui hérite de HALComponent
public:
    PotentiometerDriver(const char* name = "PotentiometerDriver");
    ~PotentiometerDriver() override;

    ErrorCode initialize() override;
    ErrorCode configurePotentiometer(PotentiometerType type, uint8_t pin, int rawMin = POT_DEFAULT_ADC_MIN, int rawMax = POT_DEFAULT_ADC_MAX, float outMin = 0.0f, float outMax = 100.0f, float smoothing = POT_DEFAULT_SMOOTHING_FACTOR);

    // Lit la valeur mappée et lissée
    float getValue(PotentiometerType type);
    // Lit la valeur brute de l'ADC
    int getRawValue(PotentiometerType type);

    // Doit être appelé régulièrement pour mettre à jour les lectures (si lissage ou détection de changement)
    void update() override;

    // Méthodes d'accès aux données spécifiques
    bool dataAvailable(PotentiometerType type);

protected:
    void onEnable() override;
    void onDisable() override;
    // void run() override; // Si ce composant a sa propre tâche pour la lecture

private:
    PotentiometerConfig _potConfigs[MAX_POTENTIOMETERS];
    uint8_t _numConfiguredPots;
    bool _isInitialized;
    // Configuration ADC globale (résolution, atténuation pour ESP32) si nécessaire
    // static bool _adcInitialized; // Pour initialiser l'ADC une seule fois
};
