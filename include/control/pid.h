#ifndef PID_H
#define PID_H

class PIDController {
public:
    void init(float kp, float ki, float kd, float maxOutput, float minOutput, float maxIntegral);
    float compute(float setpoint, float measurement, float deltaTime);

private:
    float kp;
    float ki;
    float kd;
    float maxOutput;
    float minOutput;
    float maxIntegral;
    float lastError;
    float integral;
};

#endif // PID_H