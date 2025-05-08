#ifndef WINCH_H
#define WINCH_H

#include <Arduino.h>

class Winch {
public:
    Winch();
    void init();
    void control(float tension);
    void stop();
    void update();
    void checkHealth(); // Vérifie l'état de santé du treuil
private:
    float currentTension;
};

#endif // WINCH_H
