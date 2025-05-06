#pragma once

#include <Arduino.h>

class ButtonUIManager {
public:
    ButtonUIManager(DisplayManager* displayManager);
    void begin();
    void checkButtons();

private:
    DisplayManager* displayManager;
};