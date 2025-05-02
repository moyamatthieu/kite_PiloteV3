// presenter.cpp
#include "../include/presenter.h"

Presenter::Presenter(DisplayManager* display, ButtonUIManager* buttonUI) 
    : display(display), buttonUI(buttonUI) {}

void Presenter::updateDisplayWithPotValues(int direction, int trim, int lineLength) {
    if (display->getCurrentDisplayState() == DISPLAY_DIRECTION_TRIM) {
        display->updateDirectionTrimDisplay(direction, trim);
    } else if (display->getCurrentDisplayState() == DISPLAY_LINE_LENGTH) {
        display->updateLineLengthDisplay(lineLength);
    }
}

void Presenter::showMenu(MenuState menu) {
    display->showMenu(menu);
}

void Presenter::handleButtonPress(uint8_t buttonId) {
    // Logique de gestion des pressions sur les boutons
    if (buttonId == BUTTON_BLUE) {
        display->nextScreen();
    } else if (buttonId == BUTTON_GREEN) {
        display->showMenu(MENU_MAIN);
    } else if (buttonId == BUTTON_RED) {
        // Appliquer les valeurs actuelles des potentiomètres aux moteurs
        // Cette logique pourrait être déplacée vers le Controller
    }
}