#ifndef GENERATOR_H
#define GENERATOR_H

#include <Arduino.h>

class Generator {
public:
    Generator();
    void init();
    void control(float power);
    void stop();
    void update();
private:
    float currentPower;
};

#endif // GENERATOR_H
