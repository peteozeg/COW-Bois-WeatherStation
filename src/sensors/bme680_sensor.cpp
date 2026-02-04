/**
 * COW-Bois Weather Station - BME680 Sensor Implementation
 */

#include "sensors/bme680_sensor.h"
#include "config.h"

BME680Sensor::BME680Sensor()
    : _initialized(false)
    , _tempOffset(0)
    , _humidityOffset(0)
    , _pressureOffset(0)
    , _lastTemp(0)
    , _lastHumidity(0)
    , _lastPressure(0)
    , _lastGasResistance(0)
    , _readingValid(false) {
}

bool BME680Sensor::begin(uint8_t addr) {
    DEBUG_PRINTF("BME680: Initializing at address 0x%02X\n", addr);

    if (!_bme.begin(addr)) {
        DEBUG_PRINTLN("BME680: Failed to initialize");
        _initialized = false;
        return false;
    }

    // Configure for weather monitoring
    _bme.setTemperatureOversampling(BME680_OS_8X);
    _bme.setHumidityOversampling(BME680_OS_2X);
    _bme.setPressureOversampling(BME680_OS_4X);
    _bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    _bme.setGasHeater(320, 150);  // 320°C for 150ms

    _initialized = true;
    DEBUG_PRINTLN("BME680: Initialized successfully");
    return true;
}

bool BME680Sensor::performReading() {
    if (!_initialized) {
        _readingValid = false;
        return false;
    }

    if (!_bme.performReading()) {
        DEBUG_PRINTLN("BME680: Failed to perform reading");
        _readingValid = false;
        return false;
    }

    _lastTemp = _bme.temperature + _tempOffset;
    _lastHumidity = _bme.humidity + _humidityOffset;

    // Clamp humidity to valid range
    if (_lastHumidity < 0) _lastHumidity = 0;
    if (_lastHumidity > 100) _lastHumidity = 100;

    // Convert Pa to hPa and apply offset
    _lastPressure = (_bme.pressure / 100.0f) + _pressureOffset;

    // Gas resistance in KOhms
    _lastGasResistance = _bme.gas_resistance / 1000.0f;

    _readingValid = true;
    return true;
}

bool BME680Sensor::isConnected() {
    if (!_initialized) return false;
    return performReading();
}

float BME680Sensor::readTemperature() {
    if (!_initialized) return NAN;

    if (!performReading()) return NAN;
    return _lastTemp;
}

float BME680Sensor::readHumidity() {
    if (!_initialized) return NAN;

    if (!performReading()) return NAN;
    return _lastHumidity;
}

float BME680Sensor::readPressure() {
    if (!_initialized) return NAN;

    if (!performReading()) return NAN;
    return _lastPressure;
}

float BME680Sensor::readGasResistance() {
    if (!_initialized) return NAN;

    if (!performReading()) return NAN;
    return _lastGasResistance;
}

bool BME680Sensor::readAll(float& temp, float& humidity, float& pressure) {
    if (!_initialized) {
        temp = NAN;
        humidity = NAN;
        pressure = NAN;
        return false;
    }

    if (!performReading()) {
        temp = NAN;
        humidity = NAN;
        pressure = NAN;
        return false;
    }

    temp = _lastTemp;
    humidity = _lastHumidity;
    pressure = _lastPressure;

    return true;
}

bool BME680Sensor::readAllWithGas(float& temp, float& humidity, float& pressure, float& gasResistance) {
    if (!_initialized) {
        temp = NAN;
        humidity = NAN;
        pressure = NAN;
        gasResistance = NAN;
        return false;
    }

    if (!performReading()) {
        temp = NAN;
        humidity = NAN;
        pressure = NAN;
        gasResistance = NAN;
        return false;
    }

    temp = _lastTemp;
    humidity = _lastHumidity;
    pressure = _lastPressure;
    gasResistance = _lastGasResistance;

    return true;
}
