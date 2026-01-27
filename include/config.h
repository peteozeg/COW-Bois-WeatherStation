/**
 * COW-Bois Weather Station - Configuration
 * Central configuration constants for the weather station firmware
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// Firmware Version
// ============================================
#define FIRMWARE_VERSION "1.0.0"
#define FIRMWARE_DATE "2026-01-26"

// ============================================
// Timing Configuration (per AASC standards)
// ============================================
#define SAMPLE_INTERVAL_MS 3000        // Sample sensors every 3 seconds
#define TRANSMIT_INTERVAL_MS 300000    // Transmit data every 5 minutes
#define AGGREGATION_WINDOW_MS 300000   // Aggregation window (same as transmit)
#define ESPNOW_TRANSMIT_INTERVAL_MS 30000  // Microstation transmit interval
#define SENSOR_WARMUP_MS 2000          // Sensor warmup time after init
#define WARMUP_TIME_MS 15000           // SGP30 warmup time
#define MODEM_TIMEOUT_MS 30000         // Cellular modem timeout
#define MQTT_KEEPALIVE_S 60            // MQTT keepalive interval

// ============================================
// MQTT Configuration
// ============================================
#define MQTT_PORT 1883
#define MQTT_SECURE_PORT 8883
#define MQTT_TOPIC_PREFIX "cowbois/weather"
#define MQTT_TOPIC_DATA "cowbois/weather/data"
#define MQTT_TOPIC_STATUS "cowbois/weather/status"
#define MQTT_TOPIC_COMMAND "cowbois/weather/command"
#define MQTT_CLIENT_ID_PREFIX "cowbois_"
#define MQTT_QOS 1
#define MQTT_RETAIN false
#define MQTT_MAX_PACKET_SIZE 1024
#define MQTT_RECONNECT_INTERVAL 5000

// ============================================
// ESP-NOW Configuration
// ============================================
#define ESPNOW_CHANNEL 1
#define ESPNOW_MAX_PEERS 10
#define ESPNOW_RETRY_COUNT 3
#define ESPNOW_RETRY_DELAY_MS 100
#define ESPNOW_MAX_PACKET_SIZE 250     // ESP-NOW max payload

// ============================================
// Sensor Accuracy Thresholds (Mesonet standards)
// ============================================
#define TEMP_ACCURACY_C 0.4f           // ±0.3°C to ±0.4°C
#define HUMIDITY_ACCURACY_PCT 3.0f     // ±2% to ±3%
#define PRESSURE_ACCURACY_MB 1.0f      // ±1mb
#define WIND_SPEED_ACCURACY_MS 0.3f    // ±0.3 m/s
#define WIND_DIR_ACCURACY_DEG 3.0f     // ±3°
#define PRECIP_ACCURACY_PCT 5.0f       // ±1% to ±5%
#define SOLAR_ACCURACY_PCT 5.0f        // ±5%
#define SOLAR_RESOLUTION_WM2 0.2f      // 0.2 W/m²
#define PRECIP_RESOLUTION_MM 0.254f    // 0.254mm

// ============================================
// Data Aggregation
// ============================================
#define MAX_SAMPLES_PER_INTERVAL 100   // 5 min / 3 sec = 100 samples
#define DATA_BUFFER_SIZE 10            // Store last 10 aggregated readings

// ============================================
// Power Management
// ============================================
#define BATTERY_LOW_VOLTAGE 3.3f       // Low battery warning threshold
#define BATTERY_CRITICAL_VOLTAGE 3.0f  // Critical - enter deep sleep
#define BATTERY_FULL_VOLTAGE 4.2f      // Fully charged voltage
#define BATTERY_EMPTY_VOLTAGE 3.0f     // Empty battery voltage
#define BATTERY_VOLTAGE_DIVIDER 2.0f   // Voltage divider ratio
#define SOLAR_MIN_VOLTAGE 5.0f         // Minimum solar panel voltage

// ============================================
// Sensor Configuration
// ============================================
#define LUX_TO_WM2 0.0079f             // Lux to W/m² conversion factor
#define PRECIP_CALIBRATION_FACTOR 420.0f  // HX711 calibration factor
#define PRECIP_COLLECTOR_AREA 50.0f    // Rain gauge collector area (cm²)

// ============================================
// Cellular Modem Configuration
// ============================================
#define MODEM_BAUD_RATE 115200

// ============================================
// Debug Configuration
// ============================================
#define DEBUG_ENABLED true
#define DEBUG_BAUD_RATE 115200
#define DEBUG_PRINT_INTERVAL_MS 1000   // Print debug info every second

// Debug macros
#if DEBUG_ENABLED
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
#endif

// ============================================
// Station Identification
// ============================================
#define STATION_ID_LENGTH 16
#define DEFAULT_STATION_NAME "COW-Bois-001"

#endif // CONFIG_H
