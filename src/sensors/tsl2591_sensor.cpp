/**
 * COW-Bois Weather Station - TSL2591 Sensor Implementation
 */

#include "sensors/tsl2591_sensor.h"
#include "config.h"

TSL2591Sensor::TSL2591Sensor()
    : _tsl(2591)  // Sensor ID
    , _initialized(false)
    , _calibrationFactor(1.0f) {
}

bool TSL2591Sensor::begin() {
    DEBUG_PRINTLN("TSL2591: Initializing...");

    if (!_tsl.begin()) {
        DEBUG_PRINTLN("TSL2591: Failed to initialize");
        _initialized = false;
        return false;
    }

    // Configure sensor for outdoor use
    // Medium gain and 100ms integration for versatility
    _tsl.setGain(TSL2591_GAIN_MED);
    _tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);

    _initialized = true;
    DEBUG_PRINTLN("TSL2591: Initialized successfully");
    return true;
}

bool TSL2591Sensor::isConnected() {
    if (!_initialized) return false;

    // Try reading the sensor to verify connection
    // If the sensor is disconnected, getFullLuminosity() returns 0xFFFFFFFF
    uint32_t lum = _tsl.getFullLuminosity();
    return (lum != 0xFFFFFFFF);
}

uint32_t TSL2591Sensor::readLux() {
    if (!_initialized) return 0;

    uint32_t lum = _tsl.getFullLuminosity();
    uint16_t ir = lum >> 16;
    uint16_t full = lum & 0xFFFF;

    float lux = _tsl.calculateLux(full, ir);

    // Handle overflow or invalid readings
    if (lux < 0 || isnan(lux) || isinf(lux)) {
        return 0;
    }

    return (uint32_t)lux;
}

bool TSL2591Sensor::readRaw(uint16_t& ir, uint16_t& full) {
    if (!_initialized) {
        ir = 0;
        full = 0;
        return false;
    }

    uint32_t lum = _tsl.getFullLuminosity();
    ir = lum >> 16;
    full = lum & 0xFFFF;

    return true;
}

float TSL2591Sensor::luxToIrradiance(uint32_t lux) {
    // Approximate conversion from lux to W/m²
    // This is highly dependent on the light spectrum
    // For sunlight: ~1 W/m² ≈ 120 lux (varies 80-120 lux)
    // We use a middle value and apply calibration factor
    return (float)lux * LUX_TO_WM2 * _calibrationFactor;
}

float TSL2591Sensor::readIrradiance() {
    uint32_t lux = readLux();
    return luxToIrradiance(lux);
}

void TSL2591Sensor::setGain(tsl2591Gain_t gain) {
    if (_initialized) {
        _tsl.setGain(gain);
    }
}

void TSL2591Sensor::setIntegrationTime(tsl2591IntegrationTime_t time) {
    if (_initialized) {
        _tsl.setTiming(time);
    }
}

void TSL2591Sensor::autoGain() {
    if (!_initialized) return;

    uint32_t lux = readLux();

    // Adjust gain based on light level
    if (lux < 100) {
        _tsl.setGain(TSL2591_GAIN_MAX);
        DEBUG_PRINTLN("TSL2591: Auto-gain set to MAX");
    } else if (lux < 1000) {
        _tsl.setGain(TSL2591_GAIN_HIGH);
        DEBUG_PRINTLN("TSL2591: Auto-gain set to HIGH");
    } else if (lux < 10000) {
        _tsl.setGain(TSL2591_GAIN_MED);
        DEBUG_PRINTLN("TSL2591: Auto-gain set to MED");
    } else {
        _tsl.setGain(TSL2591_GAIN_LOW);
        DEBUG_PRINTLN("TSL2591: Auto-gain set to LOW");
    }
}
