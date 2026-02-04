/**
 * COW-Bois Weather Station - BME680 Sensor
 * Temperature, Humidity, Pressure, and Gas Resistance sensor driver
 */

#ifndef BME680_SENSOR_H
#define BME680_SENSOR_H

#include <Arduino.h>
#include <Adafruit_BME680.h>

class BME680Sensor {
public:
    BME680Sensor();

    /**
     * Initialize the BME680 sensor
     * @param addr I2C address (default 0x76)
     * @return true if initialization successful
     */
    bool begin(uint8_t addr = 0x76);

    /**
     * Check if sensor is connected and responsive
     * @return true if sensor is OK
     */
    bool isConnected();

    /**
     * Read temperature
     * @return Temperature in °C
     */
    float readTemperature();

    /**
     * Read relative humidity
     * @return Humidity in % RH
     */
    float readHumidity();

    /**
     * Read barometric pressure
     * @return Pressure in hPa (mb)
     */
    float readPressure();

    /**
     * Read gas resistance (air quality indicator)
     * @return Gas resistance in KOhms
     */
    float readGasResistance();

    /**
     * Read all values at once (more efficient)
     * @param temp Reference to store temperature
     * @param humidity Reference to store humidity
     * @param pressure Reference to store pressure
     * @return true if read successful
     */
    bool readAll(float& temp, float& humidity, float& pressure);

    /**
     * Read all values including gas resistance
     * @param temp Reference to store temperature
     * @param humidity Reference to store humidity
     * @param pressure Reference to store pressure
     * @param gasResistance Reference to store gas resistance
     * @return true if read successful
     */
    bool readAllWithGas(float& temp, float& humidity, float& pressure, float& gasResistance);

    /**
     * Get sensor status
     * @return true if sensor is operational
     */
    bool isOk() const { return _initialized; }

    /**
     * Set temperature offset for calibration
     * @param offset Offset in °C
     */
    void setTemperatureOffset(float offset) { _tempOffset = offset; }

    /**
     * Set humidity offset for calibration
     * @param offset Offset in %
     */
    void setHumidityOffset(float offset) { _humidityOffset = offset; }

    /**
     * Set pressure offset for calibration
     * @param offset Offset in hPa
     */
    void setPressureOffset(float offset) { _pressureOffset = offset; }

private:
    Adafruit_BME680 _bme;
    bool _initialized;
    float _tempOffset;
    float _humidityOffset;
    float _pressureOffset;

    // Cached readings from last performReading()
    float _lastTemp;
    float _lastHumidity;
    float _lastPressure;
    float _lastGasResistance;
    bool _readingValid;

    /**
     * Perform a reading and cache the results
     * @return true if reading successful
     */
    bool performReading();
};

#endif // BME680_SENSOR_H
