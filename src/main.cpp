/**
 * COW-Bois Remote Weather Station
 * Kansas State University - ECE 591
 *
 * Main firmware for ESP32-WROOM-32U
 *
 * Team Members:
 * - Gantzen Miller (Hardware/Mechanical)
 * - Kennedy Jones (Hardware/Mechanical)
 * - Pete Ozegovic (Software)
 * - Ben Rogers (Software/Mechanical)
 * - Christian Evans (Power System/Hardware)
 * - Abdullah Ali (Power System)
 */

#include <Arduino.h>
#include <Wire.h>

// Project includes
#include "config.h"
#include "pin_definitions.h"
#include "data/weather_data.h"

// Sensor modules
#include "sensors/sensor_manager.h"

// Communication modules
#include "communication/mqtt_handler.h"
#include "communication/espnow_handler.h"
#include "communication/cellular_modem.h"

// Data processing modules
#include "data/data_aggregator.h"
#include "data/data_formatter.h"

// System modules
#include "system/power_manager.h"
#include "system/station_mode.h"

// ============================================
// Global Objects
// ============================================

SensorManager sensors;
DataAggregator aggregator;
PowerManager power;
StationModeManager stationMode;
MQTTHandler mqtt;
ESPNowHandler espNow;
CellularModem modem;

// ============================================
// Global Variables
// ============================================

unsigned long lastSampleTime = 0;
unsigned long lastTransmitTime = 0;
unsigned long lastStatusTime = 0;

// Main station peer address (set this to your main station's MAC)
uint8_t mainStationMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ============================================
// Callback Functions
// ============================================

void onESPNowReceive(const uint8_t* mac, const uint8_t* data, int len) {
    DEBUG_PRINTF("Received ESP-NOW data from %02X:%02X:%02X:%02X:%02X:%02X\n",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // Parse incoming weather packet
    ESPNowPacket packet;
    if (espNow.parseWeatherPacket(data, len, packet)) {
        DEBUG_PRINTF("  Station: %s, Temp: %.2f°C\n",
                     packet.stationId, packet.temperature / 100.0f);

        // If we're the main station, forward this data via MQTT
        if (stationMode.isMainStation() && mqtt.isConnected()) {
            char payload[512];
            snprintf(payload, sizeof(payload),
                "{\"station_id\":\"%s\",\"temperature\":%.2f,\"humidity\":%.2f,"
                "\"pressure\":%.2f,\"wind_speed\":%.2f,\"wind_direction\":%u}",
                packet.stationId,
                packet.temperature / 100.0f,
                packet.humidity / 100.0f,
                packet.pressure / 10.0f,
                packet.windSpeed / 100.0f,
                packet.windDirection);

            char topic[64];
            snprintf(topic, sizeof(topic), "%s/%s/weather",
                     MQTT_TOPIC_PREFIX, packet.stationId);
            mqtt.publish(topic, payload);
        }
    }
}

// ============================================
// Setup
// ============================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("========================================");
    Serial.println("COW-Bois Remote Weather Station");
    Serial.println("Kansas State University - ECE 591");
    Serial.println("========================================");

    // Detect station mode
    StationMode mode = stationMode.begin(STATION_MODE_PIN);
    stationMode.printConfig();

    // Initialize I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    DEBUG_PRINTLN("I2C initialized");

    // Initialize power management
    power.begin(BATTERY_ADC_PIN, CHARGING_STATUS_PIN);
    power.printStatus();

    // Initialize sensors
    Serial.println("\nInitializing sensors...");
    if (sensors.begin()) {
        Serial.println("Sensors initialized successfully");
    } else {
        Serial.println("WARNING: Some sensors failed to initialize");
    }

    // Print sensor status
    SensorStatus status = sensors.getStatus();
    Serial.printf("  BME280: %s\n", status.bme280_ok ? "OK" : "FAILED");
    Serial.printf("  TSL2591: %s\n", status.tsl2591_ok ? "OK" : "FAILED");
    Serial.printf("  SGP30: %s\n", status.sgp30_ok ? "OK" : "FAILED");
    Serial.printf("  Wind: %s\n", status.windSensor_ok ? "OK" : "FAILED");
    Serial.printf("  Precipitation: %s\n", status.precipitation_ok ? "OK" : "FAILED");

    // Run sensor self-test
    Serial.println("\nRunning sensor self-test...");
    sensors.selfTest();

    // Initialize communication based on station mode
    Serial.println("\nInitializing communication...");

    if (stationMode.useESPNow()) {
        if (espNow.begin()) {
            Serial.println("ESP-NOW initialized");

            // Set up receive callback for main station
            if (stationMode.shouldReceiveMicrostationData()) {
                espNow.setOnReceiveCallback(onESPNowReceive);
            }

            // Microstations add main station as peer
            if (stationMode.isMicrostation()) {
                espNow.addPeer(mainStationMAC);
            }
        } else {
            Serial.println("ESP-NOW initialization failed");
        }
    }

    if (stationMode.useCellular()) {
        // Initialize cellular modem (main station only)
        Serial.println("Initializing cellular modem...");
        if (modem.begin(Serial1, MODEM_RX_PIN, MODEM_TX_PIN,
                        MODEM_POWER_PIN, MODEM_RESET_PIN)) {
            Serial.println("Modem initialized");

            // Connect to network
            #ifdef CELLULAR_APN
            if (modem.connect(CELLULAR_APN, CELLULAR_USER, CELLULAR_PASS)) {
                Serial.printf("Connected to cellular network. Signal: %d dBm\n",
                              modem.getSignalQuality());
            }
            #else
            Serial.println("WARNING: Cellular APN not configured. Check secrets.h");
            #endif
        } else {
            Serial.println("Modem initialization failed");
        }
    }

    Serial.println("\n========================================");
    Serial.println("Setup complete. Starting measurements...");
    Serial.printf("Sample interval: %lu ms\n", stationMode.getRecommendedSampleInterval());
    Serial.printf("Transmit interval: %lu ms\n", stationMode.getRecommendedTransmitInterval());
    Serial.println("========================================\n");
}

// ============================================
// Main Loop
// ============================================

void loop() {
    unsigned long currentTime = millis();

    // Handle MQTT if connected
    if (stationMode.useCellular() && mqtt.isConnected()) {
        mqtt.loop();
    }

    // Take samples at configured interval
    if (currentTime - lastSampleTime >= stationMode.getRecommendedSampleInterval()) {
        lastSampleTime = currentTime;

        // Read all sensors
        WeatherReading reading;
        if (sensors.readAll(reading)) {
            // Add to aggregator
            aggregator.addSample(reading);

            #if DEBUG_ENABLED
            DataFormatter::printReading(reading);
            #endif
        }
    }

    // Transmit aggregated data at configured interval
    if (currentTime - lastTransmitTime >= stationMode.getRecommendedTransmitInterval()) {
        lastTransmitTime = currentTime;

        // Get aggregated data
        AggregatedData data = aggregator.getAndReset();

        if (data.sampleCount > 0) {
            #if DEBUG_ENABLED
            DataFormatter::printAggregated(data);
            #endif

            // Transmit based on station mode
            if (stationMode.isMicrostation()) {
                // Send via ESP-NOW to main station
                WeatherReading lastReading;
                lastReading.timestamp = data.timestamp;
                lastReading.temperature = data.tempAvg;
                lastReading.humidity = data.humidityAvg;
                lastReading.pressure = data.pressureAvg;
                lastReading.windSpeed = data.windSpeedAvg;
                lastReading.windDirection = data.windDirAvg;
                lastReading.precipitation = data.precipitation;
                lastReading.lux = data.luxAvg;
                lastReading.solarIrradiance = data.solarAvg;
                lastReading.co2 = data.co2Avg;
                lastReading.tvoc = data.tvocAvg;
                lastReading.isValid = true;

                if (espNow.sendWeatherData(mainStationMAC, lastReading)) {
                    DEBUG_PRINTLN("Data sent via ESP-NOW");
                } else {
                    DEBUG_PRINTLN("ESP-NOW transmission failed");
                }
            } else if (stationMode.isMainStation()) {
                // Send via MQTT (through cellular modem)
                char payload[1024];
                DataFormatter::toMQTTPayload(stationMode.getStationId(), data,
                                             payload, sizeof(payload));

                char topic[64];
                snprintf(topic, sizeof(topic), "%s/%s/weather",
                         MQTT_TOPIC_PREFIX, stationMode.getStationId());

                if (mqtt.isConnected()) {
                    mqtt.publish(topic, payload);
                    DEBUG_PRINTLN("Data sent via MQTT");
                } else {
                    DEBUG_PRINTLN("MQTT not connected, data not sent");
                }
            }
        }
    }

    // Periodic status update
    if (currentTime - lastStatusTime >= 60000) {  // Every minute
        lastStatusTime = currentTime;

        // Update battery status
        power.updateBatteryStatus();

        // Check for low battery
        if (power.isCriticalBattery()) {
            Serial.println("CRITICAL: Battery critically low!");
            // Consider entering deep sleep
        } else if (power.isLowBattery()) {
            Serial.println("WARNING: Battery low");
        }

        // Print status
        DEBUG_PRINTF("Status - Battery: %.2fV (%d%%), Samples: %u\n",
                     power.readBatteryVoltage(),
                     power.readBatteryPercent(),
                     aggregator.getSampleCount());
    }

    // Small delay to prevent tight looping
    delay(10);
}
