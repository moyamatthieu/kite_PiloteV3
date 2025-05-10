#include "../../include/control/pid.h"
#include "../../include/utils/logging.h"

void PIDController::init(float kp, float ki, float kd, float maxOutput, float minOutput, float maxIntegral) {
    this->kp = kp;
    this->ki = ki;
    this->kd = kd;
    this->maxOutput = maxOutput;
    this->minOutput = minOutput;
    this->maxIntegral = maxIntegral;
    this->lastError = 0;
    this->integral = 0;
    LOG_INFO("PID", "Contrôleur PID initialisé");
}

float PIDController::compute(float setpoint, float measurement, float deltaTime) {
    float error = setpoint - measurement;
    integral += error * deltaTime;
    integral = constrain(integral, -maxIntegral, maxIntegral);
    float derivative = (error - lastError) / deltaTime;
    lastError = error;
    float output = kp * error + ki * integral + kd * derivative;
    output = constrain(output, minOutput, maxOutput);
    return output;
}