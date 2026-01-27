/**
 * COW-Bois Weather Station - Station Mode Manager Implementation
 */

#include "system/station_mode.h"
#include "config.h"
#include <WiFi.h>

StationModeManager::StationModeManager()
    : _mode(StationMode::STANDALONE)
    , _modePin(0)
    , _latitude(0)
    , _longitude(0)
    , _elevation(0) {
    memset(_stationId, 0, sizeof(_stationId));
}

StationMode StationModeManager::begin(uint8_t modePin) {
    DEBUG_PRINTLN("StationMode: Initializing...");

    _modePin = modePin;

    // Configure mode selection pin
    pinMode(_modePin, INPUT_PULLUP);

    // Detect mode from hardware
    _mode = detectMode();

    // Generate default station ID
    generateDefaultId();

    DEBUG_PRINTF("StationMode: Mode detected as %s\n", getModeString());
    DEBUG_PRINTF("StationMode: Station ID: %s\n", _stationId);

    return _mode;
}

StationMode StationModeManager::detectMode() {
    // Read mode pin
    // LOW = Main station (jumper installed)
    // HIGH = Microstation (no jumper, pulled up)

    int pinState = digitalRead(_modePin);

    if (pinState == LOW) {
        return StationMode::MAIN_STATION;
    } else {
        return StationMode::MICROSTATION;
    }
}

const char* StationModeManager::getModeString() const {
    switch (_mode) {
        case StationMode::MAIN_STATION:
            return "MAIN_STATION";
        case StationMode::MICROSTATION:
            return "MICROSTATION";
        case StationMode::STANDALONE:
            return "STANDALONE";
        default:
            return "UNKNOWN";
    }
}

void StationModeManager::setStationId(const char* id) {
    strncpy(_stationId, id, STATION_ID_LENGTH);
    _stationId[STATION_ID_LENGTH] = '\0';
    DEBUG_PRINTF("StationMode: Station ID set to %s\n", _stationId);
}

void StationModeManager::generateDefaultId() {
    // Generate ID from MAC address
    uint8_t mac[6];
    WiFi.macAddress(mac);

    snprintf(_stationId, sizeof(_stationId), "WX%02X%02X%02X%02X",
             mac[2], mac[3], mac[4], mac[5]);
}

void StationModeManager::setLocation(float lat, float lon, int elevation) {
    _latitude = lat;
    _longitude = lon;
    _elevation = elevation;

    DEBUG_PRINTF("StationMode: Location set to %.6f, %.6f, %dm\n",
                 _latitude, _longitude, _elevation);
}

bool StationModeManager::useCellular() const {
    // Only main station uses cellular
    return _mode == StationMode::MAIN_STATION;
}

bool StationModeManager::useESPNow() const {
    // Microstation always uses ESP-NOW
    // Main station uses ESP-NOW to receive from microstations
    return _mode == StationMode::MICROSTATION || _mode == StationMode::MAIN_STATION;
}

bool StationModeManager::shouldReceiveMicrostationData() const {
    // Only main station receives microstation data
    return _mode == StationMode::MAIN_STATION;
}

uint32_t StationModeManager::getRecommendedSampleInterval() const {
    switch (_mode) {
        case StationMode::MAIN_STATION:
            return SAMPLE_INTERVAL_MS;  // 3 seconds
        case StationMode::MICROSTATION:
            return SAMPLE_INTERVAL_MS;  // 3 seconds
        case StationMode::STANDALONE:
            return 1000;  // 1 second for testing
        default:
            return SAMPLE_INTERVAL_MS;
    }
}

uint32_t StationModeManager::getRecommendedTransmitInterval() const {
    switch (_mode) {
        case StationMode::MAIN_STATION:
            return TRANSMIT_INTERVAL_MS;  // 5 minutes
        case StationMode::MICROSTATION:
            return ESPNOW_TRANSMIT_INTERVAL_MS;  // 30 seconds via ESP-NOW
        case StationMode::STANDALONE:
            return 10000;  // 10 seconds for testing
        default:
            return TRANSMIT_INTERVAL_MS;
    }
}

void StationModeManager::printConfig() const {
    Serial.println("=== Station Configuration ===");
    Serial.printf("Mode: %s\n", getModeString());
    Serial.printf("Station ID: %s\n", _stationId);
    Serial.printf("Location: %.6f, %.6f\n", _latitude, _longitude);
    Serial.printf("Elevation: %d m\n", _elevation);
    Serial.printf("Use Cellular: %s\n", useCellular() ? "Yes" : "No");
    Serial.printf("Use ESP-NOW: %s\n", useESPNow() ? "Yes" : "No");
    Serial.printf("Receive Microstation Data: %s\n",
                  shouldReceiveMicrostationData() ? "Yes" : "No");
    Serial.printf("Sample Interval: %lu ms\n", getRecommendedSampleInterval());
    Serial.printf("Transmit Interval: %lu ms\n", getRecommendedTransmitInterval());
    Serial.println("=============================");
}

