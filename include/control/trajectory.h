#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <Arduino.h>

class Trajectory {
public:
    Trajectory();
    void init();
    void calculate(float windSpeed, float lineLength);
    void update();
    float getTargetPosition(int index);
private:
    float targetPositions[3];
};

#endif // TRAJECTORY_H
