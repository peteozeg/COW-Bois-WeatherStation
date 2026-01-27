/**
 * COW-Bois Weather Station - Power Manager Implementation
 */

#include "system/power_manager.h"
#include "config.h"
#include <esp_sleep.h>

PowerManager::PowerManager()
    : _initialized(false)
    , _batteryPin(0)
    , _chargingPin(0)
    , _currentState(PowerState::NORMAL)
    , _lastBatteryVoltage(0)
    , _lastBatteryPercent(0) {
}

bool PowerManager::begin(uint8_t batteryPin, uint8_t chargingPin) {
    DEBUG_PRINTLN("PowerManager: Initializing...");

    _batteryPin = batteryPin;
    _chargingPin = chargingPin;

    // Configure ADC for battery monitoring
    pinMode(_batteryPin, INPUT);
    analogReadResolution(12);  // 12-bit ADC
    analogSetAttenuation(ADC_11db);  // Full range 0-3.3V

    // Configure charging status pin (if available)
    if (_chargingPin != 255) {
        pinMode(_chargingPin, INPUT_PULLUP);
    }

    // Take initial reading
    updateBatteryStatus();

    _initialized = true;
    DEBUG_PRINTLN("PowerManager: Initialized successfully");
    DEBUG_PRINTF("PowerManager: Battery: %.2fV (%d%%)\n",
                 _lastBatteryVoltage, _lastBatteryPercent);

    return true;
}

float PowerManager::readBatteryVoltage() {
    if (!_initialized) return 0;

    // Read ADC value (average multiple readings for stability)
    uint32_t adcSum = 0;
    for (int i = 0; i < 10; i++) {
        adcSum += analogRead(_batteryPin);
        delay(1);
    }
    uint16_t adcValue = adcSum / 10;

    // Convert ADC value to voltage
    // ESP32 ADC: 0-4095 = 0-3.3V
    // Battery voltage divider: typically 2:1 ratio (100k:100k)
    // So actual battery voltage = measured * 2

    float measuredVoltage = (adcValue / 4095.0f) * 3.3f;
    float batteryVoltage = measuredVoltage * BATTERY_VOLTAGE_DIVIDER;

    _lastBatteryVoltage = batteryVoltage;
    return batteryVoltage;
}

uint8_t PowerManager::readBatteryPercent() {
    float voltage = readBatteryVoltage();

    // Convert voltage to percentage
    // LiPo battery: ~4.2V full, ~3.0V empty
    // Using a simple linear approximation (could be improved with discharge curve)

    uint8_t percent;
    if (voltage >= BATTERY_FULL_VOLTAGE) {
        percent = 100;
    } else if (voltage <= BATTERY_EMPTY_VOLTAGE) {
        percent = 0;
    } else {
        percent = (uint8_t)((voltage - BATTERY_EMPTY_VOLTAGE) /
                           (BATTERY_FULL_VOLTAGE - BATTERY_EMPTY_VOLTAGE) * 100);
    }

    _lastBatteryPercent = percent;
    return percent;
}

bool PowerManager::isCharging() const {
    if (_chargingPin == 255) return false;

    // Most charging ICs pull the status pin LOW when charging
    return digitalRead(_chargingPin) == LOW;
}

bool PowerManager::isLowBattery() {
    return _lastBatteryVoltage < BATTERY_LOW_VOLTAGE;
}

bool PowerManager::isCriticalBattery() {
    return _lastBatteryVoltage < BATTERY_CRITICAL_VOLTAGE;
}

void PowerManager::updateBatteryStatus() {
    readBatteryVoltage();
    readBatteryPercent();

    // Update power state based on battery level
    if (isCriticalBattery()) {
        _currentState = PowerState::CRITICAL;
        DEBUG_PRINTLN("PowerManager: CRITICAL battery level!");
    } else if (isLowBattery()) {
        _currentState = PowerState::LOW_POWER;
        DEBUG_PRINTLN("PowerManager: Low battery warning");
    } else {
        _currentState = PowerState::NORMAL;
    }
}

PowerState PowerManager::getState() const {
    return _currentState;
}

void PowerManager::enterDeepSleep(uint32_t sleepTimeSeconds) {
    DEBUG_PRINTF("PowerManager: Entering deep sleep for %lu seconds\n", sleepTimeSeconds);

    // Configure wake up source
    esp_sleep_enable_timer_wakeup(sleepTimeSeconds * 1000000ULL);

    // Optional: Configure GPIO wake up
    // esp_sleep_enable_ext0_wakeup(GPIO_NUM_X, 1);

    // Enter deep sleep
    esp_deep_sleep_start();
}

void PowerManager::enterLightSleep(uint32_t sleepTimeMs) {
    DEBUG_PRINTF("PowerManager: Entering light sleep for %lu ms\n", sleepTimeMs);

    // Configure wake up source
    esp_sleep_enable_timer_wakeup(sleepTimeMs * 1000ULL);

    // Enter light sleep
    esp_light_sleep_start();

    DEBUG_PRINTLN("PowerManager: Woke from light sleep");
}

void PowerManager::enterModemSleep() {
    DEBUG_PRINTLN("PowerManager: Entering modem sleep");

    // Disable WiFi and Bluetooth to save power
    // WiFi.disconnect(true);
    // WiFi.mode(WIFI_OFF);
    // btStop();

    _currentState = PowerState::SLEEP;
}

uint32_t PowerManager::getRecommendedSleepTime() const {
    // Adjust sleep time based on battery level
    switch (_currentState) {
        case PowerState::CRITICAL:
            return 3600;  // 1 hour - conserve as much as possible
        case PowerState::LOW_POWER:
            return 600;   // 10 minutes
        case PowerState::NORMAL:
        default:
            return 300;   // 5 minutes (normal transmit interval)
    }
}

void PowerManager::printStatus() const {
    Serial.println("=== Power Status ===");
    Serial.printf("Battery: %.2f V (%d%%)\n", _lastBatteryVoltage, _lastBatteryPercent);
    Serial.printf("Charging: %s\n", isCharging() ? "Yes" : "No");
    Serial.printf("State: %s\n", getStateString());
    Serial.println("====================");
}

const char* PowerManager::getStateString() const {
    switch (_currentState) {
        case PowerState::NORMAL:
            return "NORMAL";
        case PowerState::LOW_POWER:
            return "LOW_POWER";
        case PowerState::SLEEP:
            return "SLEEP";
        case PowerState::CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

void PowerManager::setLowPowerMode(bool enable) {
    if (enable) {
        // Reduce CPU frequency to save power
        setCpuFrequencyMhz(80);
        DEBUG_PRINTLN("PowerManager: Low power mode enabled (80MHz)");
    } else {
        // Restore normal CPU frequency
        setCpuFrequencyMhz(240);
        DEBUG_PRINTLN("PowerManager: Normal power mode (240MHz)");
    }
}

float PowerManager::estimateRuntimeHours() const {
    // Very rough estimate based on battery percentage
    // Assumes constant current draw (not realistic but useful approximation)
    // Typical draw: ~100mA active, ~10mA sleep

    // Average current assuming 90% sleep time
    float avgCurrentMa = 0.9f * 10.0f + 0.1f * 100.0f;  // ~19mA

    // Battery capacity (typical 18650 = 2600mAh)
    float capacityMah = 2600.0f * (_lastBatteryPercent / 100.0f);

    return capacityMah / avgCurrentMa;
}

