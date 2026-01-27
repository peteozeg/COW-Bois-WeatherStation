/**
 * COW-Bois Weather Station - Power Manager
 * Battery monitoring and power management
 */

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "pin_definitions.h"

// Power states
enum class PowerState {
    NORMAL,         // Normal operation
    LOW_POWER,      // Battery below threshold, reduce activity
    SLEEP,          // In sleep mode
    CRITICAL        // Battery critical, prepare for shutdown
};

class PowerManager {
public:
    PowerManager();

    /**
     * Initialize power manager
     * @param batteryPin ADC pin for battery voltage
     * @param chargingPin Pin to detect charging (255 = not used)
     * @return true if initialization successful
     */
    bool begin(uint8_t batteryPin = BATTERY_ADC_PIN, uint8_t chargingPin = 255);

    /**
     * Read battery voltage
     * @return Battery voltage in volts
     */
    float readBatteryVoltage();

    /**
     * Get battery percentage
     * @return Battery level 0-100%
     */
    uint8_t readBatteryPercent();

    /**
     * Check if battery is charging
     * @return true if charging detected
     */
    bool isCharging() const;

    /**
     * Check if battery is low
     * @return true if battery below low threshold
     */
    bool isLowBattery();

    /**
     * Check if battery is critical
     * @return true if battery below critical threshold
     */
    bool isCriticalBattery();

    /**
     * Update battery status (call periodically)
     */
    void updateBatteryStatus();

    /**
     * Get current power state
     * @return PowerState enum value
     */
    PowerState getState() const;

    /**
     * Enter deep sleep mode
     * @param sleepTimeSeconds Sleep duration in seconds
     */
    void enterDeepSleep(uint32_t sleepTimeSeconds);

    /**
     * Enter light sleep mode
     * @param sleepTimeMs Sleep duration in milliseconds
     */
    void enterLightSleep(uint32_t sleepTimeMs);

    /**
     * Enter modem sleep mode (disable WiFi/BT)
     */
    void enterModemSleep();

    /**
     * Get recommended sleep time based on battery level
     * @return Recommended sleep time in seconds
     */
    uint32_t getRecommendedSleepTime() const;

    /**
     * Enable or disable low power mode (reduce CPU frequency)
     * @param enable true to enable low power mode
     */
    void setLowPowerMode(bool enable);

    /**
     * Estimate remaining runtime
     * @return Estimated runtime in hours
     */
    float estimateRuntimeHours() const;

    /**
     * Print status to Serial
     */
    void printStatus() const;

    /**
     * Get power state as string
     * @return State description
     */
    const char* getStateString() const;

private:
    bool _initialized;
    uint8_t _batteryPin;
    uint8_t _chargingPin;
    PowerState _currentState;
    float _lastBatteryVoltage;
    uint8_t _lastBatteryPercent;
};

#endif // POWER_MANAGER_H
