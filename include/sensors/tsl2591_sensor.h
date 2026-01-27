/**
 * COW-Bois Weather Station - TSL2591 Sensor
 * Solar radiation / light sensor driver
 */

#ifndef TSL2591_SENSOR_H
#define TSL2591_SENSOR_H

#include <Arduino.h>
#include <Adafruit_TSL2591.h>
#include "config.h"

class TSL2591Sensor {
public:
    TSL2591Sensor();

    /**
     * Initialize the TSL2591 sensor
     * @return true if initialization successful
     */
    bool begin();

    /**
     * Check if sensor is connected and responsive
     * @return true if sensor is OK
     */
    bool isConnected();

    /**
     * Read luminosity in lux
     * @return Luminosity in lux
     */
    uint32_t readLux();

    /**
     * Read raw luminosity values
     * @param ir Reference to store IR value
     * @param full Reference to store full spectrum value
     * @return true if read successful
     */
    bool readRaw(uint16_t& ir, uint16_t& full);

    /**
     * Convert lux to solar irradiance (W/m²)
     * Note: This is an approximation. For accurate solar radiation,
     * a dedicated pyranometer is recommended.
     * @param lux Lux value to convert
     * @return Estimated irradiance in W/m²
     */
    float luxToIrradiance(uint32_t lux);

    /**
     * Read solar irradiance directly
     * @return Estimated irradiance in W/m²
     */
    float readIrradiance();

    /**
     * Set sensor gain
     * @param gain TSL2591_GAIN_LOW, _MED, _HIGH, or _MAX
     */
    void setGain(tsl2591Gain_t gain);

    /**
     * Set integration time
     * @param time TSL2591_INTEGRATIONTIME_100MS to _600MS
     */
    void setIntegrationTime(tsl2591IntegrationTime_t time);

    /**
     * Auto-adjust gain based on light level
     */
    void autoGain();

    /**
     * Get sensor status
     * @return true if sensor is operational
     */
    bool isOk() const { return _initialized; }

    /**
     * Set calibration factor for irradiance conversion
     * @param factor Calibration factor (default 1.0)
     */
    void setCalibrationFactor(float factor) { _calibrationFactor = factor; }

private:
    Adafruit_TSL2591 _tsl;
    bool _initialized;
    float _calibrationFactor;
};

#endif // TSL2591_SENSOR_H
