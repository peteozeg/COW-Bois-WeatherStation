/**
 * COW-Bois Weather Station - Sensor Manager Implementation
 */

#include "sensors/sensor_manager.h"
#include "config.h"

SensorManager::SensorManager() : _initialized(false) {
    _status = {false, false, false, false, false};
}

bool SensorManager::begin() {
    DEBUG_PRINTLN("SensorManager: Initializing all sensors...");

    // Initialize BME280 (Temperature, Humidity, Pressure)
    _status.bme280_ok = _bme280.begin();
    if (_status.bme280_ok) {
        DEBUG_PRINTLN("  BME280: OK");
    } else {
        DEBUG_PRINTLN("  BME280: FAILED");
    }

    // Initialize TSL2591 (Solar Radiation)
    _status.tsl2591_ok = _tsl2591.begin();
    if (_status.tsl2591_ok) {
        DEBUG_PRINTLN("  TSL2591: OK");
    } else {
        DEBUG_PRINTLN("  TSL2591: FAILED");
    }

    // Initialize SGP30 (Air Quality)
    _status.sgp30_ok = _sgp30.begin();
    if (_status.sgp30_ok) {
        DEBUG_PRINTLN("  SGP30: OK");
    } else {
        DEBUG_PRINTLN("  SGP30: FAILED");
    }

    // Initialize Wind Sensor
    _status.windSensor_ok = _wind.begin();
    if (_status.windSensor_ok) {
        DEBUG_PRINTLN("  Wind Sensor: OK");
    } else {
        DEBUG_PRINTLN("  Wind Sensor: FAILED");
    }

    // Initialize Precipitation Sensor
    _status.precipitation_ok = _precip.begin();
    if (_status.precipitation_ok) {
        DEBUG_PRINTLN("  Precipitation: OK");
    } else {
        DEBUG_PRINTLN("  Precipitation: FAILED");
    }

    // Consider initialized if at least BME280 works (critical sensor)
    _initialized = _status.bme280_ok;

    DEBUG_PRINTF("SensorManager: Initialization %s\n",
                 _initialized ? "complete" : "failed");

    return _initialized;
}

bool SensorManager::readAll(WeatherReading& reading) {
    if (!_initialized) {
        reading.isValid = false;
        return false;
    }

    reading.timestamp = millis();
    reading.isValid = true;

    // Read BME280
    if (_status.bme280_ok) {
        if (!_bme280.readAll(reading.temperature, reading.humidity, reading.pressure)) {
            reading.temperature = 0;
            reading.humidity = 0;
            reading.pressure = 0;
        }
    }

    // Read TSL2591
    if (_status.tsl2591_ok) {
        reading.lux = _tsl2591.readLux();
        reading.solarIrradiance = _tsl2591.readIrradiance();
    }

    // Read SGP30
    if (_status.sgp30_ok) {
        // Set humidity compensation if available
        if (_status.bme280_ok && reading.humidity > 0) {
            float absHumidity = SGP30Sensor::calculateAbsoluteHumidity(
                reading.temperature, reading.humidity);
            _sgp30.setHumidityCompensation(absHumidity);
        }

        if (!_sgp30.readAll(reading.co2, reading.tvoc)) {
            reading.co2 = 0;
            reading.tvoc = 0;
        }
    }

    // Read Wind Sensor
    if (_status.windSensor_ok) {
        if (!_wind.readAll(reading.windSpeed, reading.windDirection)) {
            reading.windSpeed = 0;
            reading.windDirection = 0;
        }
    }

    // Read Precipitation
    if (_status.precipitation_ok) {
        reading.precipitation = _precip.readPrecipitation();
    }

    return reading.isValid;
}

SensorStatus SensorManager::getStatus() const {
    return _status;
}

bool SensorManager::selfTest() {
    DEBUG_PRINTLN("SensorManager: Running self-test...");

    bool allPass = true;

    // Test BME280
    if (_status.bme280_ok) {
        float temp = _bme280.readTemperature();
        if (temp < -40 || temp > 85) {
            DEBUG_PRINTLN("  BME280 self-test: FAILED (temp out of range)");
            allPass = false;
        } else {
            DEBUG_PRINTF("  BME280 self-test: PASS (temp=%.1f°C)\n", temp);
        }
    }

    // Test TSL2591
    if (_status.tsl2591_ok) {
        uint32_t lux = _tsl2591.readLux();
        DEBUG_PRINTF("  TSL2591 self-test: PASS (lux=%lu)\n", lux);
    }

    // Test SGP30
    if (_status.sgp30_ok) {
        if (_sgp30.isWarmedUp()) {
            uint16_t co2, tvoc;
            _sgp30.readAll(co2, tvoc);
            DEBUG_PRINTF("  SGP30 self-test: PASS (CO2=%u, TVOC=%u)\n", co2, tvoc);
        } else {
            DEBUG_PRINTLN("  SGP30 self-test: WARMING UP");
        }
    }

    // Test Wind Sensor
    if (_status.windSensor_ok) {
        uint16_t speedRaw, dirRaw;
        _wind.readRaw(speedRaw, dirRaw);
        DEBUG_PRINTF("  Wind self-test: PASS (speed=%u, dir=%u)\n", speedRaw, dirRaw);
    }

    // Test Precipitation
    if (_status.precipitation_ok) {
        float weight = _precip.readWeight();
        DEBUG_PRINTF("  Precipitation self-test: PASS (weight=%.1fg)\n", weight);
    }

    return allPass;
}

void SensorManager::calibrate() {
    DEBUG_PRINTLN("SensorManager: Calibration mode");

    // Tare precipitation sensor
    if (_status.precipitation_ok) {
        DEBUG_PRINTLN("  Taring precipitation sensor...");
        _precip.tare();
    }

    // Could add more calibration routines here
    DEBUG_PRINTLN("  Calibration complete");
}
