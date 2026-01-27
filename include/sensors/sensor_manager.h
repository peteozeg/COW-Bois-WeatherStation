/**
 * COW-Bois Weather Station - Sensor Manager
 * Unified interface for all weather sensors
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "data/weather_data.h"
#include "sensors/bme280_sensor.h"
#include "sensors/tsl2591_sensor.h"
#include "sensors/sgp30_sensor.h"
#include "sensors/wind_sensor.h"
#include "sensors/precipitation.h"

class SensorManager {
public:
    SensorManager();

    /**
     * Initialize all sensors
     * @return true if all critical sensors initialized successfully
     */
    bool begin();

    /**
     * Read all sensors and populate a WeatherReading struct
     * @param reading Reference to WeatherReading to populate
     * @return true if at least critical sensors read successfully
     */
    bool readAll(WeatherReading& reading);

    /**
     * Get status of all sensors
     * @return SensorStatus struct with individual sensor states
     */
    SensorStatus getStatus() const;

    /**
     * Run sensor self-test
     * @return true if all sensors pass self-test
     */
    bool selfTest();

    /**
     * Calibrate sensors that support calibration
     */
    void calibrate();

    // Individual sensor access (for testing)
    BME280Sensor& getBME280() { return _bme280; }
    TSL2591Sensor& getTSL2591() { return _tsl2591; }
    SGP30Sensor& getSGP30() { return _sgp30; }
    WindSensor& getWindSensor() { return _wind; }
    PrecipitationSensor& getPrecipitation() { return _precip; }

private:
    BME280Sensor _bme280;
    TSL2591Sensor _tsl2591;
    SGP30Sensor _sgp30;
    WindSensor _wind;
    PrecipitationSensor _precip;

    SensorStatus _status;
    bool _initialized;
};

#endif // SENSOR_MANAGER_H
