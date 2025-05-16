#include "control/pid.h" // Chemin relatif corrigé
#include "core/logging.h"   // Chemin relatif corrigé
#include <Arduino.h>         // Pour constrain et fabs

// Constructeur par défaut
PIDController::PIDController() 
    : kp_(0.0f), ki_(0.0f), kd_(0.0f), setpoint_(0.0f),
      max_output_(100.0f), min_output_(-100.0f), 
      max_integral_(10.0f), integral_windup_limit_(0.0f), // integral_windup_limit_ à 0 désactive cette fonctionnalité par défaut
      last_error_(0.0f), integral_sum_(0.0f),
      proportional_term_(0.0f), integral_term_(0.0f), derivative_term_(0.0f) {
    // LOG_INFO("PID", "Contrôleur PID créé avec les valeurs par défaut.");
}

void PIDController::init(float kp, float ki, float kd, float setpoint, float maxOutput, float minOutput, float maxIntegral, float integralWindupLimit) {
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
    setpoint_ = setpoint;
    max_output_ = maxOutput;
    min_output_ = minOutput;
    max_integral_ = maxIntegral; // Limite absolue pour la somme intégrale
    integral_windup_limit_ = integralWindupLimit; // Seuil pour une gestion d'anti-windup plus fine si nécessaire
    
    reset(); // Réinitialise l'état intégral et la dernière erreur
    LOG_INFO("PID", "Contrôleur PID initialisé: Kp=%.2f, Ki=%.2f, Kd=%.2f, Setpoint=%.2f", kp_, ki_, kd_, setpoint_);
}

float PIDController::compute(float measurement, float deltaTime) {
    if (deltaTime <= 0.0f) {
        LOG_WARNING("PID", "DeltaTime est nul ou négatif (%.4f). Calcul non effectué.", deltaTime);
        return proportional_term_ + integral_term_ + derivative_term_; 
    }

    float error = setpoint_ - measurement;

    // Terme Proportionnel
    proportional_term_ = kp_ * error;

    // Terme Intégral avec anti-windup
    integral_sum_ += ki_ * error * deltaTime;
    integral_sum_ = constrain(integral_sum_, -max_integral_, max_integral_);
    integral_term_ = integral_sum_;

    // Terme Dérivé
    derivative_term_ = kd_ * (error - last_error_) / deltaTime;

    // Sauvegarde de l'erreur pour la prochaine itération
    last_error_ = error;

    // Calcul de la sortie totale et clampage
    float output = proportional_term_ + integral_term_ + derivative_term_;
    output = constrain(output, min_output_, max_output_);

    return output;
}

void PIDController::reset() {
    last_error_ = 0.0f;
    integral_sum_ = 0.0f;
    proportional_term_ = 0.0f;
    integral_term_ = 0.0f;
    derivative_term_ = 0.0f;
    LOG_INFO("PID", "Contrôleur PID réinitialisé.");
}

void PIDController::setTunings(float kp, float ki, float kd) {
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
    LOG_INFO("PID", "Gains PID mis à jour: Kp=%.2f, Ki=%.2f, Kd=%.2f", kp_, ki_, kd_);
}

void PIDController::setKp(float kp) {
    kp_ = kp;
    LOG_INFO("PID", "Kp mis à jour: %.2f", kp_);
}

void PIDController::setKi(float ki) {
    ki_ = ki;
    LOG_INFO("PID", "Ki mis à jour: %.2f", ki_);
}

void PIDController::setKd(float kd) {
    kd_ = kd;
    LOG_INFO("PID", "Kd mis à jour: %.2f", kd_);
}

void PIDController::setOutputLimits(float minOutput, float maxOutput) {
    min_output_ = minOutput;
    max_output_ = maxOutput;
    LOG_INFO("PID", "Limites de sortie PID mises à jour: Min=%.2f, Max=%.2f", min_output_, max_output_);
}

void PIDController::setSetpoint(float setpoint) {
    setpoint_ = setpoint;
    LOG_INFO("PID", "Consigne PID mise à jour: %.2f", setpoint_);
}

float PIDController::getSetpoint() const {
    return setpoint_;
}

float PIDController::getProportionalTerm() const {
    return proportional_term_;
}

float PIDController::getIntegralTerm() const {
    return integral_term_;
}

float PIDController::getDerivativeTerm() const {
    return derivative_term_;
}

float PIDController::getError() const {
    return last_error_;
}