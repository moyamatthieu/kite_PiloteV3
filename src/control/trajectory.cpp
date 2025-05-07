#include "control/trajectory.h"

Trajectory::Trajectory() {
    targetPositions[0] = 0.0;
    targetPositions[1] = 0.0;
    targetPositions[2] = 0.0;
}

void Trajectory::init() {
    // Initialisation de la trajectoire
    // Code d'initialisation ici
}

void Trajectory::calculate(float windSpeed, float lineLength) {
    // Calcul de la trajectoire en fonction de la vitesse du vent et de la longueur de ligne
    // Code de calcul de trajectoire ici
}

void Trajectory::update() {
    // Mise à jour de la trajectoire
    // Code de mise à jour ici
}

float Trajectory::getTargetPosition(int index) {
    if (index >= 0 && index < 3) {
        return targetPositions[index];
    }
    return 0.0;
}
