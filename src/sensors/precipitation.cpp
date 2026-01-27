/**
 * COW-Bois Weather Station - Precipitation Sensor Implementation
 * HX711 load cell for rain gauge
 */

#include "sensors/precipitation.h"
#include "config.h"

PrecipitationSensor::PrecipitationSensor()
    : _initialized(false)
    , _calibrationFactor(PRECIP_CALIBRATION_FACTOR)
    , _tareOffset(0)
    , _collectorArea(PRECIP_COLLECTOR_AREA)
    , _lastWeight(0) {
}

bool PrecipitationSensor::begin(uint8_t dataPin, uint8_t clockPin) {
    DEBUG_PRINTLN("Precipitation: Initializing HX711...");

    _dataPin = dataPin;
    _clockPin = clockPin;

    _hx711.begin(_dataPin, _clockPin);

    // Wait for HX711 to stabilize
    delay(100);

    if (!_hx711.is_ready()) {
        DEBUG_PRINTLN("Precipitation: HX711 not ready");
        _initialized = false;
        return false;
    }

    // Set scale factor
    _hx711.set_scale(_calibrationFactor);

    // Tare the scale
    DEBUG_PRINTLN("Precipitation: Taring scale...");
    _hx711.tare(10);  // Average 10 readings for tare

    _tareOffset = _hx711.get_offset();
    _initialized = true;

    DEBUG_PRINTLN("Precipitation: Initialized successfully");
    return true;
}

bool PrecipitationSensor::isConnected() {
    if (!_initialized) return false;

    return _hx711.is_ready();
}

float PrecipitationSensor::readWeight() {
    if (!_initialized) return 0;

    if (!_hx711.is_ready()) {
        return _lastWeight;  // Return last known value
    }

    // Average multiple readings for stability
    float weight = _hx711.get_units(5);

    // Filter out negative values (noise)
    if (weight < 0) weight = 0;

    _lastWeight = weight;
    return weight;
}

float PrecipitationSensor::readPrecipitation() {
    float weight = readWeight();

    // Convert weight to precipitation depth
    // Precipitation (mm) = Volume (ml) / Area (cm²) * 10
    // Volume (ml) = Weight (g) / Density (g/ml) ≈ Weight (assuming water density ≈ 1)
    // So: Precipitation (mm) = Weight (g) / Area (cm²) * 10

    float precipitation = (weight / _collectorArea) * 10.0f;

    // Clamp to reasonable values
    if (precipitation < 0) precipitation = 0;
    if (precipitation > 500) precipitation = 500;  // Max 500mm seems reasonable

    return precipitation;
}

void PrecipitationSensor::tare() {
    if (!_initialized) return;

    DEBUG_PRINTLN("Precipitation: Taring...");

    if (_hx711.is_ready()) {
        _hx711.tare(10);
        _tareOffset = _hx711.get_offset();
        _lastWeight = 0;
        DEBUG_PRINTLN("Precipitation: Tare complete");
    } else {
        DEBUG_PRINTLN("Precipitation: HX711 not ready for tare");
    }
}

void PrecipitationSensor::calibrate(float knownWeightGrams) {
    if (!_initialized) return;

    DEBUG_PRINTF("Precipitation: Calibrating with %.1fg reference\n", knownWeightGrams);

    if (!_hx711.is_ready()) {
        DEBUG_PRINTLN("Precipitation: HX711 not ready for calibration");
        return;
    }

    // Get raw reading
    long rawReading = _hx711.get_value(10);

    if (rawReading != 0 && knownWeightGrams > 0) {
        _calibrationFactor = (float)rawReading / knownWeightGrams;
        _hx711.set_scale(_calibrationFactor);

        DEBUG_PRINTF("Precipitation: Calibration factor set to %.2f\n", _calibrationFactor);
    }
}

void PrecipitationSensor::setCalibrationFactor(float factor) {
    _calibrationFactor = factor;
    if (_initialized) {
        _hx711.set_scale(_calibrationFactor);
    }
    DEBUG_PRINTF("Precipitation: Calibration factor set to %.2f\n", _calibrationFactor);
}

void PrecipitationSensor::setCollectorArea(float areaCm2) {
    _collectorArea = areaCm2;
    DEBUG_PRINTF("Precipitation: Collector area set to %.1f cm²\n", _collectorArea);
}

bool PrecipitationSensor::checkForRain(float thresholdMm) {
    float precip = readPrecipitation();
    return precip >= thresholdMm;
}

void PrecipitationSensor::powerDown() {
    if (_initialized) {
        _hx711.power_down();
        DEBUG_PRINTLN("Precipitation: HX711 powered down");
    }
}

void PrecipitationSensor::powerUp() {
    if (_initialized) {
        _hx711.power_up();
        delay(100);  // Allow stabilization
        DEBUG_PRINTLN("Precipitation: HX711 powered up");
    }
}

float PrecipitationSensor::getRawReading() {
    if (!_initialized || !_hx711.is_ready()) {
        return 0;
    }

    return (float)_hx711.read();
}

