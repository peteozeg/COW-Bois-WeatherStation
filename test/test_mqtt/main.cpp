/**
 * COW-Bois Weather Station - MQTT Test
 *
 * Interactive test sketch to verify MQTT communication with a broker.
 * Uses the actual MQTTHandler class from the project to validate the
 * production code path.
 *
 * Requirements:
 * - WiFi network (use phone hotspot if on campus)
 * - MQTT broker running (Mosquitto on Raspberry Pi)
 * - secrets.h configured with WiFi and broker credentials
 *
 * Upload: pio run -e test_mqtt -t upload
 * Monitor: pio device monitor
 */

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "secrets.h"
#include "communication/mqtt_handler.h"
#include "data/weather_data.h"

// ============================================
// Global State
// ============================================
MQTTHandler mqtt;
bool wifiConnected = false;
uint32_t messagesPublished = 0;
uint32_t messagesReceived = 0;

// Station ID for testing
const char* testStationId = "TEST001";

// ============================================
// MQTT Callback
// ============================================
void onMqttMessage(const char* topic, const char* message) {
    messagesReceived++;
    Serial.println();
    Serial.println(F("========== MESSAGE RECEIVED =========="));
    Serial.printf("Topic: %s\n", topic);
    Serial.printf("Message: %s\n", message);
    Serial.println(F("======================================"));
    Serial.println();
}

// ============================================
// Helper Functions
// ============================================
void printHelp() {
    Serial.println();
    Serial.println(F("========== MQTT Test Commands =========="));
    Serial.println(F("  c - Connect to WiFi"));
    Serial.println(F("  m - Connect to MQTT broker"));
    Serial.println(F("  d - Disconnect from MQTT"));
    Serial.println(F("  t - Publish TEST message"));
    Serial.println(F("  w - Publish WEATHER data"));
    Serial.println(F("  s - Subscribe to command topic"));
    Serial.println(F("  x - Show status"));
    Serial.println(F("  h - Show this help"));
    Serial.println(F("========================================="));
    Serial.println();
}

void printStatus() {
    Serial.println();
    Serial.println(F("--- Current Status ---"));

    // WiFi status
    Serial.printf("WiFi: %s\n", wifiConnected ? "Connected" : "Disconnected");
    if (wifiConnected) {
        Serial.printf("  SSID: %s\n", WiFi.SSID().c_str());
        Serial.printf("  IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("  RSSI: %d dBm\n", WiFi.RSSI());
    }

    // MQTT status
    Serial.printf("MQTT: %s\n", mqtt.isConnected() ? "Connected" : "Disconnected");
    Serial.printf("  Broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);
    Serial.printf("  State: %s (%d)\n", mqtt.getStateString(), mqtt.getState());

    // Statistics
    Serial.printf("Messages Published: %u\n", messagesPublished);
    Serial.printf("Messages Received: %u\n", messagesReceived);
    Serial.println();
}

void connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("WiFi already connected"));
        return;
    }

    Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println(F("WiFi connected!"));
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Signal strength: %d dBm\n", WiFi.RSSI());
    } else {
        Serial.println(F("WiFi connection FAILED"));
        Serial.println(F("Check WIFI_SSID and WIFI_PASSWORD in secrets.h"));
    }
}

void connectMQTT() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("ERROR: Connect to WiFi first (press 'c')"));
        return;
    }

    if (mqtt.isConnected()) {
        Serial.println(F("MQTT already connected"));
        return;
    }

    Serial.printf("Connecting to MQTT broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);

    // Set credentials if provided
    #if defined(MQTT_USERNAME) && defined(MQTT_PASSWORD)
    if (strlen(MQTT_USERNAME) > 0) {
        mqtt.setCredentials(MQTT_USERNAME, MQTT_PASSWORD);
    }
    #endif

    // Set callback before connecting
    mqtt.setCallback(onMqttMessage);

    if (mqtt.begin(MQTT_BROKER, MQTT_PORT)) {
        Serial.println(F("MQTT connected!"));
    } else {
        Serial.printf("MQTT connection FAILED: %s\n", mqtt.getStateString());
        Serial.println(F("Check MQTT_BROKER in secrets.h"));
        Serial.println(F("Ensure Mosquitto is running on Pi"));
    }
}

void disconnectMQTT() {
    mqtt.disconnect();
    Serial.println(F("MQTT disconnected"));
}

void publishTestMessage() {
    if (!mqtt.isConnected()) {
        Serial.println(F("ERROR: Connect to MQTT first (press 'm')"));
        return;
    }

    char topic[64];
    snprintf(topic, sizeof(topic), "%s/test", MQTT_TOPIC_PREFIX);

    char payload[128];
    snprintf(payload, sizeof(payload),
             "{\"message\":\"Hello from COW-Bois!\",\"timestamp\":%lu}",
             millis());

    Serial.printf("Publishing to: %s\n", topic);
    Serial.printf("Payload: %s\n", payload);

    if (mqtt.publish(topic, payload, false)) {
        messagesPublished++;
        Serial.println(F("SUCCESS"));
    } else {
        Serial.println(F("FAILED"));
    }
}

void publishWeatherData() {
    if (!mqtt.isConnected()) {
        Serial.println(F("ERROR: Connect to MQTT first (press 'm')"));
        return;
    }

    // Create test weather reading with dummy data
    WeatherReading reading;
    reading.timestamp = millis();
    reading.temperature = 23.5f;
    reading.humidity = 65.0f;
    reading.pressure = 1013.25f;
    reading.gasResistance = 150.0f;
    reading.windSpeed = 3.5f;
    reading.windDirection = 225;
    reading.precipitation = 0.0f;
    reading.lux = 45000;
    reading.solarIrradiance = 355.5f;
    reading.co2 = 420;
    reading.tvoc = 50;
    reading.isValid = true;

    Serial.printf("Publishing weather data for station: %s\n", testStationId);

    if (mqtt.publishWeatherData(testStationId, reading)) {
        messagesPublished++;
        Serial.println(F("SUCCESS"));
        Serial.println(F("Data: temp=23.5C, humidity=65%, pressure=1013.25hPa"));
    } else {
        Serial.println(F("FAILED"));
    }
}

void subscribeToCommands() {
    if (!mqtt.isConnected()) {
        Serial.println(F("ERROR: Connect to MQTT first (press 'm')"));
        return;
    }

    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%s/command", MQTT_TOPIC_PREFIX, testStationId);

    Serial.printf("Subscribing to: %s\n", topic);

    if (mqtt.subscribe(topic)) {
        Serial.println(F("Subscribed! Waiting for messages..."));
        Serial.println(F("Test from Pi with:"));
        Serial.printf("  mosquitto_pub -h localhost -t \"%s\" -m \"status\"\n", topic);
    } else {
        Serial.println(F("Subscribe FAILED"));
    }
}

// ============================================
// Setup and Loop
// ============================================
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println(F("========================================="));
    Serial.println(F("    COW-Bois MQTT Communication Test"));
    Serial.println(F("========================================="));
    Serial.println();
    Serial.printf("Broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);
    Serial.printf("WiFi SSID: %s\n", WIFI_SSID);
    Serial.println();
    Serial.println(F("Steps to test:"));
    Serial.println(F("  1. Press 'c' to connect to WiFi"));
    Serial.println(F("  2. Press 'm' to connect to MQTT broker"));
    Serial.println(F("  3. Press 't' or 'w' to publish messages"));
    Serial.println();
    Serial.println(F("On Raspberry Pi, monitor with:"));
    Serial.println(F("  mosquitto_sub -h localhost -t \"cowbois/#\" -v"));
    Serial.println();

    printHelp();
}

void loop() {
    // Process MQTT messages
    if (mqtt.isConnected()) {
        mqtt.loop();
    }

    // Handle serial commands
    if (Serial.available()) {
        char cmd = Serial.read();

        // Consume extra characters (newlines, etc.)
        while (Serial.available()) {
            char c = Serial.read();
            if (c != '\r' && c != '\n') break;
        }

        switch (cmd) {
            case 'c':
            case 'C':
                connectWiFi();
                break;

            case 'm':
            case 'M':
                connectMQTT();
                break;

            case 'd':
            case 'D':
                disconnectMQTT();
                break;

            case 't':
            case 'T':
                publishTestMessage();
                break;

            case 'w':
            case 'W':
                publishWeatherData();
                break;

            case 's':
            case 'S':
                subscribeToCommands();
                break;

            case 'x':
            case 'X':
                printStatus();
                break;

            case 'h':
            case 'H':
            case '?':
                printHelp();
                break;

            default:
                break;
        }
    }

    // Update connection status for printStatus()
    wifiConnected = (WiFi.status() == WL_CONNECTED);
}
