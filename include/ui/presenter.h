// presenter.h
#ifndef PRESENTER_H
#define PRESENTER_H

#include "display.h"
#include "button_ui.h"

class Presenter {
public:
    Presenter(DisplayManager* display, ButtonUIManager* buttonUI);
    void updateDisplayWithPotValues(int direction, int trim, int lineLength);
    void showMenu(MenuState menu);
    void handleButtonPress(uint8_t buttonId);

private:
    DisplayManager* display;
    ButtonUIManager* buttonUI;
};

#endif // PRESENTER_H