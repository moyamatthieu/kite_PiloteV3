#pragma once

#include "hal/hal_component.h"
#include "common/global_enums.h"
#include <Arduino.h> // Pour millis(), attachInterrupt(), etc.

// Structure pour les données traitées du capteur de longueur de ligne
struct ProcessedLineLengthData {
    float length;           // Longueur de ligne en mètres (m)
    float speed;            // Vitesse de déroulement/enroulement en m/s (positif pour déroulement)
    long pulseCount;        // Nombre total d'impulsions depuis le dernier reset ou initialisation
    uint32_t timestamp;     // Horodatage de la mesure
    bool isValid;           // Indique si les données sont valides

    ProcessedLineLengthData() :
        length(0.0f), speed(0.0f), pulseCount(0), timestamp(0), isValid(false) {}
};

// Structure pour la configuration du capteur de longueur de ligne (encodeur rotatif)
struct LineLengthDriverConfig {
    uint8_t encoderPinA;        // Broche A de l'encodeur
    uint8_t encoderPinB;        // Broche B de l'encodeur (optionnel, pour la direction)
    float pulsesPerRevolution;  // Nombre d'impulsions par tour complet de l'encodeur
    float drumCircumference;    // Circonférence du tambour d'enroulement en mètres
    // float gearRatio;         // Rapport de réduction entre l'encodeur et le tambour (si applicable)
    bool reverseDirection;      // Inverser la direction de comptage

    LineLengthDriverConfig() :
        encoderPinA(0xFF), // Invalide par défaut
        encoderPinB(0xFF), // Invalide par défaut (ou non utilisé)
        pulsesPerRevolution(100.0f), // Exemple
        drumCircumference(0.1f),   // Exemple: 10 cm
        reverseDirection(false) {}
};

class LineLengthDriver : public SensorComponent {
public:
    LineLengthDriver(const char* name = "LineLengthDriver");
    ~LineLengthDriver() override;

    ErrorCode initialize() override;
    bool read() override; // Calcule la longueur et la vitesse à partir des impulsions
    ErrorCode configure(const LineLengthDriverConfig& config);

    // Méthodes spécifiques
    ErrorCode resetLength(float currentLength = 0.0f); // Réinitialise la longueur actuelle
    bool selfTest(); // Si un auto-test est possible

    const ProcessedLineLengthData& getProcessedData() const { return _processedData; }

    // Méthode statique pour la routine d'interruption (ISR)
    // Doit être accessible globalement ou via un pointeur statique vers l'instance
    static void IRAM_ATTR encoderISR(); 

    void readSensor(); // Ajout pour correspondance avec le .cpp

    // Méthodes d'accès aux données spécifiques
    bool dataAvailable() const;
    float getLineLength() const;
    float getLineSpeed() const;

protected:
    void onEnable() override;
    void onDisable() override;

private:
    ErrorCode applyConfig();
    void calculateMetrics(); // Calcule la longueur et la vitesse

    LineLengthDriverConfig _config;
    ProcessedLineLengthData _processedData;

    // Variables pour l'ISR et le calcul
    // `volatile` car modifié par ISR et lu par le thread principal
    static volatile long _isrPulseCount; 
    static volatile int8_t _isrLastEncoderAPinState; // Pour détection de front sur PinA
    // Pour encodeur quadrature (PinA et PinB)
    // static volatile int _isrEncoderPos;
    // static volatile uint8_t _isrEncoderState;

    long _lastPulseCountForSpeed;
    uint32_t _lastSpeedCalcTime;

    static LineLengthDriver* _instance; // Pointeur vers l'instance unique pour l'ISR
};
