#ifndef SYSTEM_ORCHESTRATOR_H
#define SYSTEM_ORCHESTRATOR_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "core/component.h"
#include "common/global_enums.h" // Pour ErrorCode, TaskPriority
#include "core/logging.h"
#include "utils/error_handler.h"
#include "core/config.h" // Pour SYSTEM_VERSION, WDT_DEFAULT_TIMEOUT_SECONDS, etc.
#include "core/system_state_manager.h" // <<< AJOUTÉ

// Structure pour les informations système
typedef struct {
    uint32_t uptimeSeconds;
    uint32_t freeHeapBytes;
    uint8_t cpuUsagePercent;    // TODO: Implémenter une vraie mesure
    float cpuTemperature;       // TODO: Implémenter une vraie mesure si capteur dispo
    uint16_t batteryVoltageMv;  // TODO: Lire depuis un capteur ADC
    uint8_t batteryPercent;     // TODO: Calculer à partir de batteryVoltageMv
} SystemInfo_t;

// SystemMode local est supprimé, utiliser SystemState de SystemStateManager

class SystemOrchestrator : public ManagedComponent {
public:
    // Singleton instance accessor
    static SystemOrchestrator* getInstance();

    // Deleted copy constructor and assignment operator
    SystemOrchestrator(const SystemOrchestrator&) = delete;
    SystemOrchestrator& operator=(const SystemOrchestrator&) = delete;

    // Overridden virtual methods from ManagedComponent
    ErrorCode initialize() override; // Changement du type de retour
    void run(); // Suppression de override
    bool shutdown(); // Suppression de override

    // Public interface
    SystemInfo_t getSystemInfo() const;
    void requestSystemRestart(unsigned long delayMs = 0);
    bool isSystemHealthy() const; // Basé sur l'état du composant et les erreurs critiques

    // Gestion des états/modes système via SystemStateManager
    bool requestSystemStateChange(SystemState newState, const char* reason = nullptr);
    SystemState getCurrentSystemState() const; // Renvoie l'état global du SystemStateManager

    // Raccourcis pour des transitions d'état courantes
    bool requestEnterPowerSaveMode(const char* reason = "Entering power save mode");
    bool requestExitPowerSaveMode(const char* reason = "Exiting power save mode");
    bool requestEnterCalibrationMode(const char* reason = "Entering calibration mode");
    bool requestExitCalibrationMode(const char* reason = "Exiting calibration mode - Ready"); // Modifié pour refléter la transition vers READY

    const char* getSystemVersion() const;
    void feedWatchdog();

private:
    // Private constructor for singleton
    SystemOrchestrator();
    ~SystemOrchestrator();

    void updateSystemInfo(); // Internal method to refresh SystemInfo_t data
    static void restartTask(void* parameters); // FreeRTOS task for delayed restart

    static SystemOrchestrator* instance;
    static SemaphoreHandle_t instanceMutex;

    SystemInfo_t currentSystemInfo;
    unsigned long lastStatusUpdateTimeMs;
    TaskHandle_t restartTaskHandle;
    bool wdtInitialized;

    // Watchdog configuration
    const uint8_t wdtTimeoutSeconds = WDT_DEFAULT_TIMEOUT_SECONDS;
};

#endif // SYSTEM_ORCHESTRATOR_H
