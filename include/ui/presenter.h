#pragma once

#include <Arduino.h>

class Presenter {
public:
    Presenter();
    void updateDisplay();
    void showMessage(const char* title, const char* message);
};