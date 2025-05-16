#!/bin/bash

# Remplacer les logPrint par LOG_INFO/ERROR/WARNING
sed -i 's/logPrint((LogLevel)LOG_LEVEL_INFO/LOG_INFO/g' src/control/autopilot.cpp
sed -i 's/logPrint((LogLevel)LOG_LEVEL_ERROR/LOG_ERROR/g' src/control/autopilot.cpp
sed -i 's/logPrint((LogLevel)LOG_LEVEL_WARNING/LOG_WARNING/g' src/control/autopilot.cpp

# Remplacer IMUData par ProcessedIMUData
sed -i 's/const IMUData&/const ProcessedIMUData&/g' src/control/autopilot.cpp

echo "Correction terminée"
