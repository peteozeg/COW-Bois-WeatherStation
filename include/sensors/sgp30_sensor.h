/**
 * COW-Bois Weather Station - SGP30 Sensor
 * Air quality sensor (CO2 and TVOC) driver
 */

#ifndef SGP30_SENSOR_H
#define SGP30_SENSOR_H

#include <Arduino.h>
#include <Adafruit_SGP30.h>
#include "config.h"

class SGP30Sensor {
public:
    SGP30Sensor();

    /**
     * Initialize the SGP30 sensor
     * @return true if initialization successful
     */
    bool begin();

    /**
     * Check if sensor is connected and responsive
     * @return true if sensor is OK
     */
    bool isConnected();

    /**
     * Read equivalent CO2 level
     * @return eCO2 in ppm
     */
    uint16_t readCO2();

    /**
     * Read Total Volatile Organic Compounds
     * @return TVOC in ppb
     */
    uint16_t readTVOC();

    /**
     * Read both CO2 and TVOC at once
     * @param co2 Reference to store CO2 value
     * @param tvoc Reference to store TVOC value
     * @return true if read successful
     */
    bool readAll(uint16_t& co2, uint16_t& tvoc);

    /**
     * Set humidity compensation for more accurate readings
     * @param humidity Absolute humidity in g/m³
     */
    void setHumidityCompensation(float humidity);

    /**
     * Calculate absolute humidity from temp and relative humidity
     * @param tempC Temperature in °C
     * @param relHumidity Relative humidity in %
     * @return Absolute humidity in g/m³
     */
    static float calculateAbsoluteHumidity(float tempC, float relHumidity);

    /**
     * Get baseline values for persistent storage
     * @param co2Baseline Reference to store CO2 baseline
     * @param tvocBaseline Reference to store TVOC baseline
     * @return true if baselines are valid
     */
    bool getBaseline(uint16_t& co2Baseline, uint16_t& tvocBaseline);

    /**
     * Set baseline values from persistent storage
     * @param co2Baseline CO2 baseline value
     * @param tvocBaseline TVOC baseline value
     */
    void setBaseline(uint16_t co2Baseline, uint16_t tvocBaseline);

    /**
     * Get sensor status
     * @return true if sensor is operational
     */
    bool isOk() const { return _initialized; }

    /**
     * Check if sensor is warmed up (needs ~15 seconds)
     * @return true if sensor is ready for accurate readings
     */
    bool isWarmedUp() const;

private:
    Adafruit_SGP30 _sgp;
    bool _initialized;
    uint32_t _initTime;
};

#endif // SGP30_SENSOR_H
