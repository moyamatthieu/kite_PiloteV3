#pragma once

#include "hal/hal_component.h" // Modifié pour inclure le bon fichier d'en-tête
#include "common/global_enums.h"
#include "core/config.h" // Pour l'adresse I2C, les dimensions, etc.
#include <LiquidCrystal_I2C.h> // Ou une autre bibliothèque d'affichage appropriée

// Structure pour la configuration du pilote d'affichage
struct DisplayDriverConfig {
    uint8_t i2cAddress;
    uint8_t columns;
    uint8_t rows;
    // Autres configurations spécifiques à l'écran (par ex. type de rétroéclairage)

    DisplayDriverConfig() :
        i2cAddress(DISPLAY_DEFAULT_I2C_ADDRESS),
        columns(DISPLAY_DEFAULT_COLUMNS),
        rows(DISPLAY_DEFAULT_ROWS) {}
};

class DisplayDriver : public OutputComponent { // Hérite de OutputComponent qui hérite de HALComponent
public:
    DisplayDriver(const char* name = "DisplayDriver");
    ~DisplayDriver() override;

    ErrorCode initialize() override;
    ErrorCode configure(const DisplayDriverConfig& config);

    ErrorCode clear();
    ErrorCode setCursor(uint8_t col, uint8_t row);
    ErrorCode print(const char* message);
    ErrorCode printLine(uint8_t row, const char* message, bool clearLine = true); // Imprime sur une ligne entière
    ErrorCode setBacklight(bool on);
    ErrorCode customChar(uint8_t location, uint8_t charmap[]);

    // Fonctions de plus haut niveau spécifiques à l'application (peuvent être déplacées vers un service UI)
    // ErrorCode displaySystemStatus(const SystemStatus& status);
    // ErrorCode displayMenu(const Menu& menu);

protected:
    void onEnable() override;
    void onDisable() override;
    // void run() override; // Si des mises à jour d'écran en arrière-plan sont nécessaires

private:
    DisplayDriverConfig _driverConfig;
    LiquidCrystal_I2C* _lcd; // Pointeur vers l'objet LCD
    bool _isInitialized;
    bool _backlightState;
};
