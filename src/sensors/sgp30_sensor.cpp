/**
 * COW-Bois Weather Station - SGP30 Sensor Implementation
 */

#include "sensors/sgp30_sensor.h"
#include "config.h"
#include <math.h>

SGP30Sensor::SGP30Sensor()
    : _initialized(false)
    , _initTime(0) {
}

bool SGP30Sensor::begin() {
    DEBUG_PRINTLN("SGP30: Initializing...");

    if (!_sgp.begin()) {
        DEBUG_PRINTLN("SGP30: Failed to initialize");
        _initialized = false;
        return false;
    }

    _initTime = millis();
    _initialized = true;

    DEBUG_PRINTLN("SGP30: Initialized successfully");
    DEBUG_PRINTLN("SGP30: Needs 15 seconds warmup for accurate readings");

    return true;
}

bool SGP30Sensor::isConnected() {
    if (!_initialized) return false;

    // SGP30 should return valid readings if connected
    return _sgp.IAQmeasure();
}

uint16_t SGP30Sensor::readCO2() {
    if (!_initialized) return 0;

    if (!_sgp.IAQmeasure()) {
        return 0;
    }

    return _sgp.eCO2;
}

uint16_t SGP30Sensor::readTVOC() {
    if (!_initialized) return 0;

    if (!_sgp.IAQmeasure()) {
        return 0;
    }

    return _sgp.TVOC;
}

bool SGP30Sensor::readAll(uint16_t& co2, uint16_t& tvoc) {
    if (!_initialized) {
        co2 = 0;
        tvoc = 0;
        return false;
    }

    if (!_sgp.IAQmeasure()) {
        co2 = 0;
        tvoc = 0;
        return false;
    }

    co2 = _sgp.eCO2;
    tvoc = _sgp.TVOC;

    return true;
}

void SGP30Sensor::setHumidityCompensation(float humidity) {
    if (!_initialized) return;

    // SGP30 expects humidity in 8.8 fixed point format
    // Value is absolute humidity in g/m³
    uint16_t humidityFixed = (uint16_t)(humidity * 256);
    _sgp.setHumidity(humidityFixed);
}

float SGP30Sensor::calculateAbsoluteHumidity(float tempC, float relHumidity) {
    // Calculate absolute humidity using the formula:
    // AH = (6.112 * e^((17.67 * T) / (T + 243.5)) * RH * 2.1674) / (273.15 + T)
    // where T is temperature in °C and RH is relative humidity in %

    float es = 6.112f * exp((17.67f * tempC) / (tempC + 243.5f));
    float ah = (es * relHumidity * 2.1674f) / (273.15f + tempC);

    return ah;  // Returns g/m³
}

bool SGP30Sensor::getBaseline(uint16_t& co2Baseline, uint16_t& tvocBaseline) {
    if (!_initialized) return false;

    return _sgp.getIAQBaseline(&co2Baseline, &tvocBaseline);
}

void SGP30Sensor::setBaseline(uint16_t co2Baseline, uint16_t tvocBaseline) {
    if (!_initialized) return;

    _sgp.setIAQBaseline(co2Baseline, tvocBaseline);
    DEBUG_PRINTF("SGP30: Baseline set to CO2=%u, TVOC=%u\n", co2Baseline, tvocBaseline);
}

bool SGP30Sensor::isWarmedUp() const {
    if (!_initialized) return false;

    return (millis() - _initTime) >= WARMUP_TIME_MS;
}
