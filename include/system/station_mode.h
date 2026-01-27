/**
 * COW-Bois Weather Station - Station Mode Manager
 * Handles main station vs microstation configuration and behavior
 */

#ifndef STATION_MODE_H
#define STATION_MODE_H

#include <Arduino.h>
#include "config.h"
#include "pin_definitions.h"

// Station modes
enum class StationMode {
    MAIN_STATION,       // Full station with cellular modem
    MICROSTATION,       // Satellite station using ESP-NOW
    STANDALONE          // Testing mode, no communication
};

class StationModeManager {
public:
    StationModeManager();

    /**
     * Initialize and detect station mode
     * @param modePin Pin to read for mode selection
     * @return Detected station mode
     */
    StationMode begin(uint8_t modePin = STATION_MODE_PIN);

    /**
     * Get current station mode
     * @return StationMode enum value
     */
    StationMode getMode() const { return _mode; }

    /**
     * Check if this is the main station
     * @return true if main station
     */
    bool isMainStation() const { return _mode == StationMode::MAIN_STATION; }

    /**
     * Check if this is a microstation
     * @return true if microstation
     */
    bool isMicrostation() const { return _mode == StationMode::MICROSTATION; }

    /**
     * Force a specific mode (for testing)
     * @param mode Mode to set
     */
    void setMode(StationMode mode) { _mode = mode; }

    /**
     * Get mode as string
     * @return Mode description string
     */
    const char* getModeString() const;

    /**
     * Set station ID
     * @param id Station identifier string
     */
    void setStationId(const char* id);

    /**
     * Get station ID
     * @return Station identifier string
     */
    const char* getStationId() const { return _stationId; }

    /**
     * Set station location
     * @param lat Latitude
     * @param lon Longitude
     * @param elevation Elevation in meters
     */
    void setLocation(float lat, float lon, int elevation);

    /**
     * Get station latitude
     * @return Latitude
     */
    float getLatitude() const { return _latitude; }

    /**
     * Get station longitude
     * @return Longitude
     */
    float getLongitude() const { return _longitude; }

    /**
     * Get station elevation
     * @return Elevation in meters
     */
    int getElevation() const { return _elevation; }

    /**
     * Should this station use cellular communication?
     * @return true if cellular should be used
     */
    bool useCellular() const;

    /**
     * Should this station use ESP-NOW?
     * @return true if ESP-NOW should be used
     */
    bool useESPNow() const;

    /**
     * Should this station receive data from microstations?
     * @return true if should receive microstation data
     */
    bool shouldReceiveMicrostationData() const;

    /**
     * Get recommended sample interval for this mode
     * @return Sample interval in milliseconds
     */
    uint32_t getRecommendedSampleInterval() const;

    /**
     * Get recommended transmit interval for this mode
     * @return Transmit interval in milliseconds
     */
    uint32_t getRecommendedTransmitInterval() const;

    /**
     * Print configuration to Serial
     */
    void printConfig() const;

private:
    StationMode _mode;
    uint8_t _modePin;
    char _stationId[STATION_ID_LENGTH + 1];
    float _latitude;
    float _longitude;
    int _elevation;

    /**
     * Detect mode from hardware pin
     */
    StationMode detectMode();

    /**
     * Generate default station ID from MAC address
     */
    void generateDefaultId();
};

#endif // STATION_MODE_H
