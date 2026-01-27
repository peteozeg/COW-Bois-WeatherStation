/**
 * COW-Bois Weather Station - Wind Sensor
 * Custom flex sensor anemometer for wind speed and direction
 */

#ifndef WIND_SENSOR_H
#define WIND_SENSOR_H

#include <Arduino.h>
#include "pin_definitions.h"

class WindSensor {
public:
    WindSensor();

    /**
     * Initialize the wind sensor
     * @param speedPin ADC pin for wind speed flex sensor
     * @param dirPin ADC pin for wind direction flex sensor
     * @return true if initialization successful
     */
    bool begin(uint8_t speedPin = WIND_SPEED_ADC_PIN,
               uint8_t dirPin = WIND_DIR_ADC_PIN);

    /**
     * Check if sensor is connected
     * @return true if sensor is operational
     */
    bool isConnected();

    /**
     * Read wind speed
     * @return Wind speed in m/s
     */
    float readWindSpeed();

    /**
     * Read wind direction
     * @return Wind direction in degrees (0-359, 0 = North)
     */
    uint16_t readWindDirection();

    /**
     * Read both speed and direction
     * @param speed Reference to store speed (m/s)
     * @param direction Reference to store direction (degrees)
     * @return true if read successful
     */
    bool readAll(float& speed, uint16_t& direction);

    /**
     * Get raw ADC values (for calibration)
     * @param speedRaw Reference to store speed ADC value
     * @param dirRaw Reference to store direction ADC value
     */
    void readRaw(uint16_t& speedRaw, uint16_t& dirRaw);

    /**
     * Calibrate wind speed at a known reference
     * @param referenceMps Known wind speed in m/s
     * @param rawReading Raw ADC value at this speed
     */
    void calibrateSpeed(float referenceMps, uint16_t rawReading);

    /**
     * Calibrate wind direction at a known reference (true north)
     * @param trueNorthRaw Raw ADC value when pointing north
     */
    void calibrateDirection(uint16_t trueNorthRaw);

    /**
     * Convert direction degrees to cardinal string
     * @param degrees Direction in degrees
     * @return Cardinal direction string (N, NE, E, etc.)
     */
    static const char* directionToCardinal(uint16_t degrees);

    /**
     * Calculate wind chill temperature
     * @param tempC Temperature in Celsius
     * @param windSpeedMps Wind speed in m/s
     * @return Wind chill temperature in Celsius
     */
    static float calculateWindChill(float tempC, float windSpeedMps);

private:
    uint8_t _speedPin;
    uint8_t _dirPin;
    bool _initialized;

    // Calibration parameters
    float _speedCalibrationFactor;
    int16_t _directionOffset;

    // Last readings cache
    uint16_t _lastSpeedRaw;
    uint16_t _lastDirRaw;
};

#endif // WIND_SENSOR_H
