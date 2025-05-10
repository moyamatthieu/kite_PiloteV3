#ifndef SAFETY_H
#define SAFETY_H

class SafetyManager {
public:
    void init();
    bool checkLimits(float value, float minLimit, float maxLimit);
    void handleSafety(float windSpeed, float lineTension, float kiteAltitude);

private:
    // Constantes pour les limites de sécurité
    static constexpr float MIN_WIND_SPEED = 5.0f;
    static constexpr float MAX_WIND_SPEED = 40.0f;
    static constexpr float MIN_LINE_TENSION = 0.0f;
    static constexpr float MAX_LINE_TENSION = 1000.0f;
    static constexpr float MIN_KITE_ALTITUDE = 0.0f;
    static constexpr float MAX_KITE_ALTITUDE = 100.0f;
};

#endif // SAFETY_H