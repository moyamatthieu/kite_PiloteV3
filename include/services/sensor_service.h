#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include "core/component.h"
#include "common/global_enums.h"
#include "core/logging.h"
#include "hal/drivers/imu_driver.h"
#include "hal/drivers/wind_driver.h"
#include "hal/drivers/tension_driver.h"
#include "hal/drivers/line_length_driver.h"
#include "hal/drivers/potentiometer_driver.h"

// Structure pour agréger toutes les données des capteurs
struct AggregatedSensorData {
    // IMU Data
    float imu_pitch, imu_roll, imu_yaw;
    float imu_accX, imu_accY, imu_accZ;
    float imu_gyroX, imu_gyroY, imu_gyroZ;
    float imu_temperature;
    bool imu_available;

    // Wind Data
    float wind_speed;
    float wind_direction; // TODO: Définir si relative ou absolue
    bool wind_sensor_available;

    // Tension Data
    float tension_left;
    float tension_right;
    bool tension_left_available;
    bool tension_right_available;

    // Line Length Data
    float line_length_left;
    float line_length_right;
    bool line_length_left_available;
    bool line_length_right_available;

    // Potentiometer Data (exemple pour deux potentiomètres)
    float pot_main_command; // Ex: Potentiomètre de commande principale
    float pot_trim;         // Ex: Potentiomètre de trim
    bool pot_main_command_available;
    bool pot_trim_available;

    unsigned long timestamp; // Millis() du moment de l'agrégation

    AggregatedSensorData() : 
        imu_pitch(0), imu_roll(0), imu_yaw(0),
        imu_accX(0), imu_accY(0), imu_accZ(0),
        imu_gyroX(0), imu_gyroY(0), imu_gyroZ(0),
        imu_temperature(0), imu_available(false),
        wind_speed(0), wind_direction(0), wind_sensor_available(false),
        tension_left(0), tension_right(0), tension_left_available(false), tension_right_available(false),
        line_length_left(0), line_length_right(0), line_length_left_available(false), line_length_right_available(false),
        pot_main_command(0), pot_trim(0), pot_main_command_available(false), pot_trim_available(false),
        timestamp(0) {}
};

class SensorService : public ManagedComponent {
public:
    SensorService(IMUDriver* imu, WindDriver* wind, 
                  TensionDriver* tensionL, TensionDriver* tensionR,
                  LineLengthDriver* lineL, LineLengthDriver* lineR,
                  PotentiometerDriver* potMain, PotentiometerDriver* potTrim);
    ~SensorService() override;

    ErrorCode initialize() override;
    void run(); // Supprimer override
    ErrorCode shutdown(); // Supprimer override

    AggregatedSensorData getAggregatedData() const;
    bool isImuDataAvailable() const;
    // Ajouter d'autres méthodes d'accès si nécessaire

private:
    void updateImuData();
    void updateWindData();
    void updateTensionData();
    void updateLineLengthData();
    void updatePotentiometerData();

    IMUDriver* imu_driver;
    WindDriver* wind_driver;
    TensionDriver* tension_left_driver;
    TensionDriver* tension_right_driver;
    LineLengthDriver* line_length_left_driver;
    LineLengthDriver* line_length_right_driver;
    PotentiometerDriver* pot_main_driver;
    PotentiometerDriver* pot_trim_driver;
    // Ajoutez d'autres pilotes de capteurs ici si nécessaire

    AggregatedSensorData current_sensor_data;
    SemaphoreHandle_t data_mutex; // Pour protéger l'accès à current_sensor_data
};

#endif // SENSOR_SERVICE_H
