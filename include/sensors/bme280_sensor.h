/**
 * COW-Bois Weather Station - BME280 Sensor
 * Temperature, Humidity, and Pressure sensor driver
 */

#ifndef BME280_SENSOR_H
#define BME280_SENSOR_H

#include <Arduino.h>
#include <Adafruit_BME280.h>

class BME280Sensor {
public:
    BME280Sensor();

    /**
     * Initialize the BME280 sensor
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
     * Read all values at once (more efficient)
     * @param temp Reference to store temperature
     * @param humidity Reference to store humidity
     * @param pressure Reference to store pressure
     * @return true if read successful
     */
    bool readAll(float& temp, float& humidity, float& pressure);

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
    Adafruit_BME280 _bme;
    bool _initialized;
    float _tempOffset;
    float _humidityOffset;
    float _pressureOffset;
};

#endif // BME280_SENSOR_H
