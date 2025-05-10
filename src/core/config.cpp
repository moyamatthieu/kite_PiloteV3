 // config.cpp

#include "core/config.h"

// Définition des variables runtime d'activation des modules
bool moduleWebserverEnabled = (MODULE_WEBSERVER_ENABLED != 0);
bool moduleWifiEnabled = (MODULE_WIFI_ENABLED != 0);
bool moduleServoEnabled = (MODULE_SERVO_ENABLED != 0);
bool moduleDisplayEnabled = (MODULE_DISPLAY_ENABLED != 0);
bool moduleApiEnabled = (MODULE_API_ENABLED != 0);
bool moduleOtaEnabled = (MODULE_OTA_ENABLED != 0);
bool moduleLoggingEnabled = (MODULE_LOGGING_ENABLED != 0);
bool moduleSensorsEnabled = (MODULE_SENSORS_ENABLED != 0);
bool moduleAutopilotEnabled = (MODULE_AUTOPILOT_ENABLED != 0);
bool moduleWinchEnabled = (MODULE_WINCH_ENABLED != 0);