/**
 * COW-Bois Weather Station - BME280 Sensor Implementation
 */

#include "sensors/bme280_sensor.h"
#include "config.h"

BME280Sensor::BME280Sensor()
    : _initialized(false)
    , _tempOffset(0)
    , _humidityOffset(0)
    , _pressureOffset(0) {
}

bool BME280Sensor::begin(uint8_t addr) {
    DEBUG_PRINTF("BME280: Initializing at address 0x%02X\n", addr);

    if (!_bme.begin(addr)) {
        DEBUG_PRINTLN("BME280: Failed to initialize");
        _initialized = false;
        return false;
    }

    // Configure for weather monitoring
    // Recommended settings from Bosch datasheet
    _bme.setSampling(
        Adafruit_BME280::MODE_NORMAL,     // Operating mode
        Adafruit_BME280::SAMPLING_X1,     // Temperature oversampling
        Adafruit_BME280::SAMPLING_X1,     // Pressure oversampling
        Adafruit_BME280::SAMPLING_X1,     // Humidity oversampling
        Adafruit_BME280::FILTER_OFF,      // Filtering
        Adafruit_BME280::STANDBY_MS_1000  // Standby time
    );

    _initialized = true;
    DEBUG_PRINTLN("BME280: Initialized successfully");
    return true;
}

bool BME280Sensor::isConnected() {
    if (!_initialized) return false;

    // Try reading to verify connection
    float temp = _bme.readTemperature();
    return !isnan(temp);
}

float BME280Sensor::readTemperature() {
    if (!_initialized) return NAN;

    float temp = _bme.readTemperature();
    if (!isnan(temp)) {
        temp += _tempOffset;
    }
    return temp;
}

float BME280Sensor::readHumidity() {
    if (!_initialized) return NAN;

    float humidity = _bme.readHumidity();
    if (!isnan(humidity)) {
        humidity += _humidityOffset;
        // Clamp to valid range
        if (humidity < 0) humidity = 0;
        if (humidity > 100) humidity = 100;
    }
    return humidity;
}

float BME280Sensor::readPressure() {
    if (!_initialized) return NAN;

    // readPressure returns Pa, convert to hPa (mb)
    float pressure = _bme.readPressure() / 100.0f;
    if (!isnan(pressure)) {
        pressure += _pressureOffset;
    }
    return pressure;
}

bool BME280Sensor::readAll(float& temp, float& humidity, float& pressure) {
    if (!_initialized) {
        temp = NAN;
        humidity = NAN;
        pressure = NAN;
        return false;
    }

    temp = readTemperature();
    humidity = readHumidity();
    pressure = readPressure();

    return !isnan(temp) && !isnan(humidity) && !isnan(pressure);
}
