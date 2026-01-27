/**
 * COW-Bois Weather Station - Wind Sensor Implementation
 * Custom flex sensor anemometer and wind vane
 */

#include "sensors/wind_sensor.h"
#include "config.h"
#include <math.h>

WindSensor::WindSensor()
    : _initialized(false)
    , _speedCalibrationFactor(1.0f)
    , _directionOffset(0)
    , _lastSpeedRaw(0)
    , _lastDirRaw(0) {
}

bool WindSensor::begin(uint8_t speedPin, uint8_t dirPin) {
    DEBUG_PRINTLN("WindSensor: Initializing...");

    _speedPin = speedPin;
    _dirPin = dirPin;

    // Configure ADC pins
    pinMode(_speedPin, INPUT);
    pinMode(_dirPin, INPUT);

    // Set ADC resolution (ESP32 default is 12-bit)
    analogReadResolution(12);

    // Take initial readings to verify sensors are connected
    uint16_t speedTest = analogRead(_speedPin);
    uint16_t dirTest = analogRead(_dirPin);

    DEBUG_PRINTF("WindSensor: Initial readings - Speed=%u, Dir=%u\n", speedTest, dirTest);

    // Check for valid readings (not at extremes which might indicate disconnection)
    if (speedTest > 50 && speedTest < 4000 && dirTest > 50 && dirTest < 4000) {
        _initialized = true;
        DEBUG_PRINTLN("WindSensor: Initialized successfully");
        return true;
    }

    // Still initialize, but warn about potentially disconnected sensors
    _initialized = true;
    DEBUG_PRINTLN("WindSensor: Warning - ADC readings at extremes, sensors may be disconnected");
    return true;
}

bool WindSensor::isConnected() {
    if (!_initialized) return false;

    // Check for reasonable ADC values
    uint16_t speed = analogRead(_speedPin);
    uint16_t dir = analogRead(_dirPin);

    // Values at extreme ends might indicate disconnection
    return (speed > 10 && speed < 4085 && dir > 10 && dir < 4085);
}

float WindSensor::readWindSpeed() {
    if (!_initialized) return 0;

    _lastSpeedRaw = analogRead(_speedPin);

    // Convert ADC value to wind speed using calibration curve
    // Flex sensor resistance changes with bending
    // This needs to be calibrated against a reference anemometer

    // Basic linear conversion (to be calibrated):
    // ADC range: 0-4095 (12-bit)
    // Assuming flex sensor gives ~1.5V at rest, ~3V at max deflection
    // Map to 0-50 m/s wind speed range

    float voltage = (_lastSpeedRaw / 4095.0f) * 3.3f;

    // Flex sensor typically has non-linear response
    // Using polynomial approximation (coefficients need calibration)
    float speed = 0;

    if (voltage > 1.5f) {
        // Above rest voltage - wind is blowing
        float deflection = voltage - 1.5f;
        // Quadratic response (typical for drag-based sensors)
        speed = 10.0f * deflection + 5.0f * deflection * deflection;
    }

    // Apply calibration factor
    speed *= _speedCalibrationFactor;

    // Clamp to reasonable range
    if (speed < 0) speed = 0;
    if (speed > 100) speed = 100;

    return speed;
}

uint16_t WindSensor::readWindDirection() {
    if (!_initialized) return 0;

    _lastDirRaw = analogRead(_dirPin);

    // Convert ADC value to direction (0-359 degrees)
    // Wind vane potentiometer gives voltage proportional to angle

    // Map ADC (0-4095) to degrees (0-359)
    uint16_t direction = map(_lastDirRaw, 0, 4095, 0, 359);

    // Apply calibration offset (to align with true north)
    direction = (direction + _directionOffset + 360) % 360;

    return direction;
}

bool WindSensor::readAll(float& speed, uint16_t& direction) {
    if (!_initialized) {
        speed = 0;
        direction = 0;
        return false;
    }

    speed = readWindSpeed();
    direction = readWindDirection();

    return true;
}

void WindSensor::readRaw(uint16_t& speedRaw, uint16_t& dirRaw) {
    if (!_initialized) {
        speedRaw = 0;
        dirRaw = 0;
        return;
    }

    speedRaw = analogRead(_speedPin);
    dirRaw = analogRead(_dirPin);

    _lastSpeedRaw = speedRaw;
    _lastDirRaw = dirRaw;
}

void WindSensor::calibrateSpeed(float referenceMps, uint16_t rawReading) {
    // Calibrate speed sensor against a reference measurement
    // referenceMps: known wind speed in m/s
    // rawReading: ADC reading at that wind speed

    if (rawReading > 0 && referenceMps > 0) {
        // Calculate what the current conversion would give
        float voltage = (rawReading / 4095.0f) * 3.3f;
        float calculatedSpeed = 0;

        if (voltage > 1.5f) {
            float deflection = voltage - 1.5f;
            calculatedSpeed = 10.0f * deflection + 5.0f * deflection * deflection;
        }

        // Adjust calibration factor
        if (calculatedSpeed > 0) {
            _speedCalibrationFactor = referenceMps / calculatedSpeed;
            DEBUG_PRINTF("WindSensor: Speed calibration factor set to %.3f\n",
                         _speedCalibrationFactor);
        }
    }
}

void WindSensor::calibrateDirection(uint16_t trueNorthRaw) {
    // Set direction offset so that trueNorthRaw reading corresponds to 0 degrees
    uint16_t rawDegrees = map(trueNorthRaw, 0, 4095, 0, 359);
    _directionOffset = (360 - rawDegrees) % 360;

    DEBUG_PRINTF("WindSensor: Direction offset set to %d degrees\n", _directionOffset);
}

const char* WindSensor::directionToCardinal(uint16_t degrees) {
    // Convert degrees to 16-point compass direction
    static const char* cardinals[] = {
        "N", "NNE", "NE", "ENE",
        "E", "ESE", "SE", "SSE",
        "S", "SSW", "SW", "WSW",
        "W", "WNW", "NW", "NNW"
    };

    // Each direction spans 22.5 degrees
    // Add 11.25 to center the ranges
    int index = ((degrees + 11) % 360) / 22;
    if (index > 15) index = 15;

    return cardinals[index];
}

float WindSensor::calculateWindChill(float tempC, float windSpeedMps) {
    // Wind chill calculation (valid for temp <= 10°C and wind >= 1.3 m/s)
    // Uses Environment Canada formula

    if (tempC > 10 || windSpeedMps < 1.3) {
        return tempC;  // Wind chill not applicable
    }

    // Convert m/s to km/h for the formula
    float windKmh = windSpeedMps * 3.6f;

    float windChill = 13.12f + 0.6215f * tempC
                    - 11.37f * pow(windKmh, 0.16f)
                    + 0.3965f * tempC * pow(windKmh, 0.16f);

    return windChill;
}

