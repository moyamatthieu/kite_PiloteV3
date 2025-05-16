#ifndef PID_H
#define PID_H

#include <Arduino.h> // Pour la fonction constrain

class PIDController {
public:
    // Constructeur par défaut
    PIDController();

    // Méthode d'initialisation (peut être appelée pour reconfigurer)
    void init(float kp, float ki, float kd, float setpoint, float maxOutput, float minOutput, float maxIntegral, float integralWindupLimit);
    
    // Calcul de la sortie du PID
    float compute(float measurement, float deltaTime);

    // Réinitialisation de l'état du PID (intégrale, dernière erreur)
    void reset();

    // Mise à jour des gains PID
    void setTunings(float kp, float ki, float kd);
    void setKp(float kp);
    void setKi(float ki);
    void setKd(float kd);

    // Mise à jour des limites de sortie
    void setOutputLimits(float minOutput, float maxOutput);

    // Mise à jour de la consigne
    void setSetpoint(float setpoint);
    float getSetpoint() const;

    // Accès aux termes individuels (utile pour le débogage ou l'analyse)
    float getProportionalTerm() const;
    float getIntegralTerm() const;
    float getDerivativeTerm() const;
    float getError() const;


private:
    float kp_; // Gain proportionnel
    float ki_; // Gain intégral
    float kd_; // Gain dérivé

    float setpoint_; // Valeur de consigne souhaitée

    float max_output_;    // Limite maximale de la sortie
    float min_output_;    // Limite minimale de la sortie
    float max_integral_;  // Limite maximale de la somme intégrale (pour anti-windup simple)
    float integral_windup_limit_; // Seuil pour l'anti-windup plus sophistiqué (si implémenté)


    float last_error_;    // Erreur précédente (pour le terme dérivé)
    float integral_sum_;  // Somme des erreurs (pour le terme intégral)

    float proportional_term_;
    float integral_term_;
    float derivative_term_;
};

#endif // PID_H