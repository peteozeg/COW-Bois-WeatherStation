/**
 * COW-Bois Weather Station - Precipitation Sensor
 * HX711 load cell based rain gauge
 */

#ifndef PRECIPITATION_H
#define PRECIPITATION_H

#include <Arduino.h>
#include <HX711.h>
#include "pin_definitions.h"
#include "config.h"

class PrecipitationSensor {
public:
    PrecipitationSensor();

    /**
     * Initialize the precipitation sensor
     * @param dataPin HX711 data pin
     * @param clockPin HX711 clock pin
     * @return true if initialization successful
     */
    bool begin(uint8_t dataPin = HX711_DOUT_PIN,
               uint8_t clockPin = HX711_SCK_PIN);

    /**
     * Check if sensor is connected
     * @return true if HX711 is ready
     */
    bool isConnected();

    /**
     * Tare (zero) the scale
     * Call this when the collection vessel is empty
     */
    void tare();

    /**
     * Read precipitation amount
     * @return Precipitation in mm
     */
    float readPrecipitation();

    /**
     * Read raw weight in grams
     * @return Weight in grams
     */
    float readWeight();

    /**
     * Get raw ADC reading
     * @return Raw HX711 reading
     */
    float getRawReading();

    /**
     * Calibrate with a known weight
     * @param knownWeightGrams Weight in grams
     */
    void calibrate(float knownWeightGrams);

    /**
     * Set calibration factor directly
     * @param factor Calibration factor
     */
    void setCalibrationFactor(float factor);

    /**
     * Get current calibration factor
     * @return Calibration factor
     */
    float getCalibrationFactor() const { return _calibrationFactor; }

    /**
     * Set collection area for mm conversion
     * @param areaCm2 Collection funnel area in cm²
     */
    void setCollectorArea(float areaCm2);

    /**
     * Check if rain is detected above threshold
     * @param thresholdMm Threshold in mm
     * @return true if precipitation exceeds threshold
     */
    bool checkForRain(float thresholdMm = 0.1f);

    /**
     * Power down the HX711 to save power
     */
    void powerDown();

    /**
     * Power up the HX711
     */
    void powerUp();

private:
    HX711 _hx711;
    bool _initialized;
    float _calibrationFactor;
    float _collectorArea;
    float _lastWeight;
    long _tareOffset;

    uint8_t _dataPin;
    uint8_t _clockPin;
};

#endif // PRECIPITATION_H
