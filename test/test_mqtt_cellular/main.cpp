/**
 * COW-Bois Weather Station - Cellular MQTT Test
 *
 * Interactive test sketch to verify MQTT communication over cellular
 * using a LILYGO T-SIM7600E board with TinyGSM library.
 *
 * Requirements:
 * - LILYGO T-SIM7600E board with SIM card installed
 * - Active T-Mobile SIM with data plan
 * - MQTT broker accessible from internet (public IP or port forwarding)
 * - secrets.h configured with CELLULAR_APN and MQTT_BROKER
 *
 * Upload: pio run -e test_mqtt_cellular -t upload
 * Monitor: pio device monitor
 */

// TinyGSM must be configured before including
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024

#include <Arduino.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include "config.h"
#include "secrets.h"
#include "data/weather_data.h"

// ============================================
// LILYGO T-SIM7600E Pin Definitions
// NOTE: Pin assignments vary between board revisions!
// If modem doesn't respond, check your specific board's pinout.
// Common alternatives: POWER_ON=12, DTR=32
// ============================================
#define MODEM_TX        26
#define MODEM_RX        27
#define MODEM_PWRKEY    4
#define MODEM_POWER_ON  25

// Serial interfaces
#define SerialMon Serial
#define SerialAT  Serial1

// ============================================
// Global Objects
// ============================================
TinyGsm modem(SerialAT);
TinyGsmClient gsmClient(modem);
PubSubClient mqtt(gsmClient);

// ============================================
// State Tracking
// ============================================
bool modemPoweredOn = false;
bool modemInitialized = false;
bool networkConnected = false;
bool gprsConnected = false;
bool mqttConnected = false;
uint32_t messagesPublished = 0;
uint32_t messagesReceived = 0;

// Station ID for testing
const char* testStationId = "TEST001";

// ============================================
// MQTT Callback
// ============================================
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    messagesReceived++;
    SerialMon.println();
    SerialMon.println(F("========== MESSAGE RECEIVED =========="));
    SerialMon.printf("Topic: %s\n", topic);
    SerialMon.print("Message: ");
    for (unsigned int i = 0; i < length; i++) {
        SerialMon.print((char)payload[i]);
    }
    SerialMon.println();
    SerialMon.println(F("======================================"));
    SerialMon.println();
}

// ============================================
// Helper Functions
// ============================================
void printHelp() {
    SerialMon.println();
    SerialMon.println(F("======== Cellular MQTT Test Commands ========"));
    SerialMon.println(F("  p - Power on modem"));
    SerialMon.println(F("  i - Initialize modem (check SIM)"));
    SerialMon.println(F("  s - Check signal strength"));
    SerialMon.println(F("  n - Connect to cellular network"));
    SerialMon.println(F("  g - Connect GPRS (data/APN)"));
    SerialMon.println(F("  m - Connect to MQTT broker"));
    SerialMon.println(F("  d - Disconnect MQTT"));
    SerialMon.println(F("  t - Publish TEST message"));
    SerialMon.println(F("  w - Publish WEATHER data"));
    SerialMon.println(F("  c - Subscribe to command topic"));
    SerialMon.println(F("  x - Show status"));
    SerialMon.println(F("  r - Reset modem"));
    SerialMon.println(F("  h - Show this help"));
    SerialMon.println(F("=============================================="));
    SerialMon.println();
    SerialMon.println(F("Typical test sequence: p -> i -> s -> n -> g -> m -> w"));
    SerialMon.println();
}

void printStatus() {
    SerialMon.println();
    SerialMon.println(F("--- Current Status ---"));

    SerialMon.printf("Modem Power:  %s\n", modemPoweredOn ? "ON" : "OFF");
    SerialMon.printf("Modem Init:   %s\n", modemInitialized ? "YES" : "NO");
    SerialMon.printf("Network:      %s\n", networkConnected ? "Connected" : "Disconnected");
    SerialMon.printf("GPRS:         %s\n", gprsConnected ? "Connected" : "Disconnected");
    SerialMon.printf("MQTT:         %s\n", mqttConnected ? "Connected" : "Disconnected");

    if (modemInitialized) {
        int16_t csq = modem.getSignalQuality();
        int dbm = (csq == 99) ? 0 : (-113 + (csq * 2));
        SerialMon.printf("  Signal: %d (CSQ), %d dBm\n", csq, dbm);
        if (gprsConnected) {
            SerialMon.printf("  IP: %s\n", modem.localIP().toString().c_str());
        }
    }

    SerialMon.printf("MQTT Broker:  %s:%d\n", MQTT_BROKER, MQTT_PORT);
    SerialMon.printf("APN:          %s\n", CELLULAR_APN);
    SerialMon.printf("Messages Published: %u\n", messagesPublished);
    SerialMon.printf("Messages Received:  %u\n", messagesReceived);
    SerialMon.println();
}

void powerOnModem() {
    SerialMon.println(F("\n--- Powering On Modem ---"));

    // Check if modem is already responding
    if (modem.testAT()) {
        SerialMon.println(F("Modem already powered on and responding!"));
        modemPoweredOn = true;
        return;
    }

    // Enable modem power supply
    SerialMon.println(F("Enabling modem power (GPIO25 HIGH)..."));
    digitalWrite(MODEM_POWER_ON, HIGH);
    delay(100);

    // Pulse PWRKEY LOW for 1 second to toggle power
    SerialMon.println(F("Pulsing PWRKEY (GPIO4 LOW for 1s)..."));
    digitalWrite(MODEM_PWRKEY, LOW);
    delay(1000);
    digitalWrite(MODEM_PWRKEY, HIGH);

    // Wait for modem to boot
    SerialMon.println(F("Waiting for modem boot (5 seconds)..."));
    delay(5000);

    // Test AT communication
    SerialMon.println(F("Testing AT communication..."));
    int attempts = 0;
    while (attempts < 10) {
        if (modem.testAT()) {
            SerialMon.println(F("Modem responding to AT commands!"));
            modemPoweredOn = true;
            return;
        }
        delay(500);
        attempts++;
        SerialMon.print(".");
    }

    SerialMon.println(F("\nFailed to communicate with modem"));
    SerialMon.println(F("Check wiring and try 'r' to reset"));
}

void initModem() {
    if (!modemPoweredOn) {
        SerialMon.println(F("ERROR: Power on modem first (press 'p')"));
        return;
    }

    SerialMon.println(F("\n--- Initializing Modem ---"));

    // Initialize modem
    SerialMon.println(F("Running modem.init()..."));
    if (!modem.init()) {
        SerialMon.println(F("Modem init failed, trying restart..."));
        if (!modem.restart()) {
            SerialMon.println(F("Modem restart failed"));
            return;
        }
    }

    // Get modem info
    String modemInfo = modem.getModemInfo();
    SerialMon.printf("Modem Info: %s\n", modemInfo.c_str());

    // Get IMEI
    String imei = modem.getIMEI();
    SerialMon.printf("IMEI: %s\n", imei.c_str());

    // Check SIM status
    SimStatus simStatus = modem.getSimStatus();
    SerialMon.print(F("SIM Status: "));
    switch (simStatus) {
        case SIM_ERROR:     SerialMon.println(F("ERROR")); break;
        case SIM_READY:     SerialMon.println(F("READY")); break;
        case SIM_LOCKED:    SerialMon.println(F("LOCKED (needs PIN)")); break;
        case SIM_ANTITHEFT_LOCKED: SerialMon.println(F("ANTITHEFT LOCKED")); break;
        default:            SerialMon.println(F("UNKNOWN")); break;
    }

    if (simStatus == SIM_READY) {
        modemInitialized = true;
        SerialMon.println(F("Modem initialized successfully!"));
    } else {
        SerialMon.println(F("SIM not ready - check SIM card"));
    }
}

void checkSignal() {
    if (!modemInitialized) {
        SerialMon.println(F("ERROR: Initialize modem first (press 'i')"));
        return;
    }

    SerialMon.println(F("\n--- Signal Strength ---"));

    int16_t signalQuality = modem.getSignalQuality();
    SerialMon.printf("Signal Quality: %d", signalQuality);

    if (signalQuality == 99) {
        SerialMon.println(F(" (Unknown/No signal)"));
    } else {
        // Convert to dBm estimate
        int dbm = -113 + (signalQuality * 2);
        SerialMon.printf(" (%d dBm) - ", dbm);

        if (signalQuality < 10) {
            SerialMon.println(F("Poor"));
        } else if (signalQuality < 15) {
            SerialMon.println(F("Fair"));
        } else if (signalQuality < 20) {
            SerialMon.println(F("Good"));
        } else {
            SerialMon.println(F("Excellent"));
        }
    }
}

void connectNetwork() {
    if (!modemInitialized) {
        SerialMon.println(F("ERROR: Initialize modem first (press 'i')"));
        return;
    }

    SerialMon.println(F("\n--- Connecting to Network ---"));
    SerialMon.println(F("Waiting for network registration (up to 60s)..."));

    if (!modem.waitForNetwork(60000)) {
        SerialMon.println(F("Network registration failed"));
        SerialMon.println(F("Check SIM card and antenna"));
        return;
    }

    if (modem.isNetworkConnected()) {
        networkConnected = true;
        String operatorName = modem.getOperator();
        SerialMon.printf("Connected to: %s\n", operatorName.c_str());
        SerialMon.println(F("Network registration successful!"));
    } else {
        SerialMon.println(F("Network connection check failed"));
    }
}

void connectGPRS() {
    if (!networkConnected) {
        SerialMon.println(F("ERROR: Connect to network first (press 'n')"));
        return;
    }

    SerialMon.println(F("\n--- Connecting GPRS ---"));
    SerialMon.printf("APN: %s\n", CELLULAR_APN);

    #if defined(CELLULAR_USER) && defined(CELLULAR_PASS)
    SerialMon.printf("User: %s\n", strlen(CELLULAR_USER) > 0 ? CELLULAR_USER : "(none)");
    if (!modem.gprsConnect(CELLULAR_APN, CELLULAR_USER, CELLULAR_PASS)) {
    #else
    if (!modem.gprsConnect(CELLULAR_APN)) {
    #endif
        SerialMon.println(F("GPRS connection failed"));
        SerialMon.println(F("Check APN settings in secrets.h"));
        return;
    }

    gprsConnected = true;
    SerialMon.println(F("GPRS connected!"));
    SerialMon.printf("IP Address: %s\n", modem.localIP().toString().c_str());
}

void connectMQTT() {
    if (!gprsConnected) {
        SerialMon.println(F("ERROR: Connect GPRS first (press 'g')"));
        return;
    }

    // Verify GPRS is still connected
    if (!modem.isGprsConnected()) {
        SerialMon.println(F("ERROR: GPRS connection lost! Reconnect with 'g'"));
        gprsConnected = false;
        return;
    }

    SerialMon.println(F("\n--- Connecting to MQTT Broker ---"));
    SerialMon.printf("Broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);

    mqtt.setServer(MQTT_BROKER, MQTT_PORT);
    mqtt.setBufferSize(MQTT_MAX_PACKET_SIZE);
    mqtt.setCallback(onMqttMessage);

    // Generate client ID from IMEI
    String imei = modem.getIMEI();
    String clientId = "cowbois-";
    if (imei.length() >= 6) {
        clientId += imei.substring(imei.length() - 6);
    } else {
        // Fallback: use millis for uniqueness
        clientId += String(millis() % 1000000);
    }
    SerialMon.printf("Client ID: %s\n", clientId.c_str());

    bool connected;
    #if defined(MQTT_USERNAME) && defined(MQTT_PASSWORD)
    if (strlen(MQTT_USERNAME) > 0) {
        SerialMon.println(F("Connecting with credentials..."));
        connected = mqtt.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD);
    } else {
        SerialMon.println(F("Connecting without credentials..."));
        connected = mqtt.connect(clientId.c_str());
    }
    #else
    SerialMon.println(F("Connecting without credentials..."));
    connected = mqtt.connect(clientId.c_str());
    #endif

    if (connected) {
        mqttConnected = true;
        SerialMon.println(F("MQTT connected!"));
    } else {
        SerialMon.printf("MQTT connection failed, state: %d\n", mqtt.state());
        SerialMon.println(F("State codes: -4=timeout, -3=connection lost, -2=connect fail"));
        SerialMon.println(F("            -1=disconnected, 0=connected, 1=bad protocol"));
        SerialMon.println(F("            2=bad client ID, 3=unavailable, 4=bad creds, 5=unauthorized"));
    }
}

void disconnectMQTT() {
    mqtt.disconnect();
    mqttConnected = false;
    SerialMon.println(F("MQTT disconnected"));
}

void publishTestMessage() {
    if (!mqttConnected || !mqtt.connected()) {
        mqttConnected = false;
        SerialMon.println(F("ERROR: Connect to MQTT first (press 'm')"));
        return;
    }

    char topic[64];
    snprintf(topic, sizeof(topic), "%s/test", MQTT_TOPIC_PREFIX);

    char payload[128];
    snprintf(payload, sizeof(payload),
             "{\"message\":\"Hello from COW-Bois Cellular!\",\"timestamp\":%lu}",
             millis());

    SerialMon.printf("Publishing to: %s\n", topic);
    SerialMon.printf("Payload: %s\n", payload);

    if (mqtt.publish(topic, payload)) {
        messagesPublished++;
        SerialMon.println(F("SUCCESS"));
    } else {
        SerialMon.println(F("FAILED"));
    }
}

void publishWeatherData() {
    if (!mqttConnected || !mqtt.connected()) {
        mqttConnected = false;
        SerialMon.println(F("ERROR: Connect to MQTT first (press 'm')"));
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

    // Build JSON payload
    char payload[512];
    snprintf(payload, sizeof(payload),
        "{"
        "\"timestamp\":%lu,"
        "\"temperature\":%.2f,"
        "\"humidity\":%.2f,"
        "\"pressure\":%.2f,"
        "\"gas_resistance\":%.2f,"
        "\"wind_speed\":%.2f,"
        "\"wind_direction\":%u,"
        "\"precipitation\":%.2f,"
        "\"lux\":%lu,"
        "\"solar_irradiance\":%.2f,"
        "\"co2\":%u,"
        "\"tvoc\":%u,"
        "\"valid\":%s"
        "}",
        reading.timestamp,
        reading.temperature,
        reading.humidity,
        reading.pressure,
        reading.gasResistance,
        reading.windSpeed,
        reading.windDirection,
        reading.precipitation,
        reading.lux,
        reading.solarIrradiance,
        reading.co2,
        reading.tvoc,
        reading.isValid ? "true" : "false"
    );

    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%s/weather", MQTT_TOPIC_PREFIX, testStationId);

    SerialMon.printf("Publishing weather data for station: %s\n", testStationId);
    SerialMon.printf("Topic: %s\n", topic);
    SerialMon.printf("Payload length: %u bytes\n", (unsigned int)strlen(payload));

    if (mqtt.publish(topic, payload)) {
        messagesPublished++;
        SerialMon.println(F("SUCCESS"));
        SerialMon.println(F("Data: temp=23.5C, humidity=65%, pressure=1013.25hPa"));
    } else {
        SerialMon.println(F("FAILED"));
    }
}

void subscribeToCommands() {
    if (!mqttConnected || !mqtt.connected()) {
        mqttConnected = false;
        SerialMon.println(F("ERROR: Connect to MQTT first (press 'm')"));
        return;
    }

    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%s/command", MQTT_TOPIC_PREFIX, testStationId);

    SerialMon.printf("Subscribing to: %s\n", topic);

    if (mqtt.subscribe(topic)) {
        SerialMon.println(F("Subscribed! Waiting for messages..."));
        SerialMon.println(F("Test from broker with:"));
        SerialMon.printf("  mosquitto_pub -h %s -t \"%s\" -m \"status\"\n", MQTT_BROKER, topic);
    } else {
        SerialMon.println(F("Subscribe FAILED"));
    }
}

void resetModem() {
    SerialMon.println(F("\n--- Resetting Modem ---"));

    // Reset state
    modemPoweredOn = false;
    modemInitialized = false;
    networkConnected = false;
    gprsConnected = false;
    mqttConnected = false;

    // Power cycle
    SerialMon.println(F("Cutting modem power..."));
    digitalWrite(MODEM_POWER_ON, LOW);
    delay(2000);

    SerialMon.println(F("Modem reset. Press 'p' to power on again."));
}

// ============================================
// Setup and Loop
// ============================================
void setup() {
    SerialMon.begin(115200);
    delay(1000);

    SerialMon.println();
    SerialMon.println(F("=============================================="));
    SerialMon.println(F("    COW-Bois Cellular MQTT Test"));
    SerialMon.println(F("    LILYGO T-SIM7600E + TinyGSM"));
    SerialMon.println(F("=============================================="));
    SerialMon.println();

    // Initialize modem control pins
    pinMode(MODEM_POWER_ON, OUTPUT);
    pinMode(MODEM_PWRKEY, OUTPUT);
    digitalWrite(MODEM_POWER_ON, LOW);
    digitalWrite(MODEM_PWRKEY, HIGH);  // Idle state

    // Initialize modem serial
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

    SerialMon.println(F("Pin Configuration:"));
    SerialMon.printf("  MODEM_TX:       GPIO %d\n", MODEM_TX);
    SerialMon.printf("  MODEM_RX:       GPIO %d\n", MODEM_RX);
    SerialMon.printf("  MODEM_PWRKEY:   GPIO %d\n", MODEM_PWRKEY);
    SerialMon.printf("  MODEM_POWER_ON: GPIO %d\n", MODEM_POWER_ON);
    SerialMon.println();

    SerialMon.printf("MQTT Broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);
    SerialMon.printf("APN: %s\n", CELLULAR_APN);
    SerialMon.println();

    SerialMon.println(F("Test sequence:"));
    SerialMon.println(F("  1. Press 'p' - Power on modem"));
    SerialMon.println(F("  2. Press 'i' - Initialize modem & check SIM"));
    SerialMon.println(F("  3. Press 's' - Check signal strength"));
    SerialMon.println(F("  4. Press 'n' - Connect to cellular network"));
    SerialMon.println(F("  5. Press 'g' - Connect GPRS (data connection)"));
    SerialMon.println(F("  6. Press 'm' - Connect to MQTT broker"));
    SerialMon.println(F("  7. Press 'w' - Publish weather data"));
    SerialMon.println();
    SerialMon.println(F("On your MQTT broker, monitor with:"));
    SerialMon.println(F("  mosquitto_sub -h localhost -t \"cowbois/#\" -v"));
    SerialMon.println();

    printHelp();
}

void loop() {
    // Process MQTT messages if connected
    if (mqttConnected) {
        if (!mqtt.connected()) {
            mqttConnected = false;
            SerialMon.println(F("MQTT connection lost!"));
        } else {
            mqtt.loop();
        }
    }

    // Handle serial commands
    if (SerialMon.available()) {
        char cmd = SerialMon.read();

        // Consume extra characters (newlines, etc.)
        while (SerialMon.available()) {
            char c = SerialMon.read();
            if (c != '\r' && c != '\n') break;
        }

        switch (cmd) {
            case 'p':
            case 'P':
                powerOnModem();
                break;

            case 'i':
            case 'I':
                initModem();
                break;

            case 's':
            case 'S':
                checkSignal();
                break;

            case 'n':
            case 'N':
                connectNetwork();
                break;

            case 'g':
            case 'G':
                connectGPRS();
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

            case 'c':
            case 'C':
                subscribeToCommands();
                break;

            case 'x':
            case 'X':
                printStatus();
                break;

            case 'r':
            case 'R':
                resetModem();
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
}
