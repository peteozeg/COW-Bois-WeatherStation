/**
 * COW-Bois Weather Station - ESP-NOW Test
 *
 * Interactive test sketch to verify ESP-NOW communication between devices.
 * Uses the actual ESPNowHandler class from the project to validate the
 * production code path.
 *
 * Upload: pio run -e test_espnow -t upload
 * Monitor: pio device monitor
 *
 * Testing requires TWO devices:
 *   Device A: Receiver mode (press 'r')
 *   Device B: Sender mode (press 's'), add Device A's MAC (press 'a')
 */

#include <Arduino.h>
#include <WiFi.h>

// Include project headers - using production code
#include "config.h"
#include "data/weather_data.h"
#include "communication/espnow_handler.h"

// ============================================
// Forward Declarations
// ============================================
void printRawData(const uint8_t* data, int len);

// ============================================
// Global State
// ============================================
ESPNowHandler espnow;  // Production ESP-NOW handler
bool isSenderMode = false;
uint8_t peerMAC[6] = {0};
bool hasPeer = false;
uint32_t packetsSent = 0;
uint32_t packetsReceived = 0;

// ============================================
// Callbacks (using ESPNowHandler signatures)
// ============================================
void onSendCallback(const uint8_t* macAddr, bool success) {
    Serial.print(F("Send to "));
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X ",
                  macAddr[0], macAddr[1], macAddr[2],
                  macAddr[3], macAddr[4], macAddr[5]);
    if (success) {
        Serial.println(F("SUCCESS"));
        packetsSent++;
    } else {
        Serial.println(F("FAILED"));
    }
}

void onReceiveCallback(const uint8_t* macAddr, const uint8_t* data, int len) {
    packetsReceived++;

    Serial.println();
    Serial.println(F("========== PACKET RECEIVED =========="));
    Serial.printf("From: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  macAddr[0], macAddr[1], macAddr[2],
                  macAddr[3], macAddr[4], macAddr[5]);
    Serial.printf("Length: %d bytes\n", len);

    // Try to parse as weather packet using production code
    ESPNowPacket packet;
    if (espnow.parseWeatherPacket(data, len, packet)) {
        Serial.println(F("Type: Weather Data Packet"));
        Serial.printf("Station ID: %s\n", packet.stationId);
        Serial.printf("Timestamp: %lu\n", packet.timestamp);
        Serial.println(F("--- Sensor Data ---"));
        Serial.printf("Temperature: %.2f C\n", packet.temperature / 100.0f);
        Serial.printf("Humidity: %.2f %%\n", packet.humidity / 100.0f);
        Serial.printf("Pressure: %.1f hPa\n", packet.pressure / 10.0f);
        Serial.printf("Gas Resistance: %.1f KOhms\n", packet.gasResistance / 10.0f);
        Serial.printf("Wind Speed: %.2f m/s\n", packet.windSpeed / 100.0f);
        Serial.printf("Wind Direction: %d deg\n", packet.windDirection);
        Serial.printf("Precipitation: %.2f mm\n", packet.precipitation / 100.0f);
        Serial.printf("Lux: %lu\n", packet.lux);
        Serial.printf("CO2: %d ppm\n", packet.co2);
        Serial.printf("TVOC: %d ppb\n", packet.tvoc);
        Serial.printf("Battery: %d mV\n", packet.batteryVoltage);
        Serial.printf("Flags: 0x%02X (valid=%d)\n", packet.flags, packet.flags & 0x01);
        Serial.println(F("Checksum: VALID"));
    } else {
        Serial.println(F("Type: Raw Data (not a valid weather packet)"));
        printRawData(data, len);
    }
    Serial.println(F("======================================"));
    Serial.println();
}

void printRawData(const uint8_t* data, int len) {
    Serial.print(F("Hex: "));
    for (int i = 0; i < len && i < 64; i++) {
        Serial.printf("%02X ", data[i]);
    }
    if (len > 64) Serial.print(F("..."));
    Serial.println();

    Serial.print(F("ASCII: "));
    for (int i = 0; i < len && i < 64; i++) {
        char c = data[i];
        Serial.print((c >= 32 && c < 127) ? c : '.');
    }
    if (len > 64) Serial.print(F("..."));
    Serial.println();
}

// ============================================
// Helper Functions
// ============================================
void printHelp() {
    Serial.println();
    Serial.println(F("========== ESP-NOW Test Commands =========="));
    Serial.println(F("  i - Initialize ESP-NOW (ESPNowHandler)"));
    Serial.println(F("  d - Deinitialize ESP-NOW"));
    Serial.println(F("  m - Show MAC address"));
    Serial.println(F("  s - Set SENDER mode"));
    Serial.println(F("  r - Set RECEIVER mode"));
    Serial.println(F("  a - Add peer (enter MAC address)"));
    Serial.println(F("  b - Add broadcast peer (FF:FF:FF:FF:FF:FF)"));
    Serial.println(F("  c - Clear peer"));
    Serial.println(F("  t - Send TEST packet (raw bytes)"));
    Serial.println(F("  w - Send WEATHER packet (via ESPNowHandler)"));
    Serial.println(F("  x - Show statistics"));
    Serial.println(F("  h - Show this help"));
    Serial.println(F("============================================="));
    Serial.println(F("NOTE: This test uses the production ESPNowHandler class"));
    Serial.println();
}

void printStatus() {
    Serial.println();
    Serial.println(F("--- Current Status ---"));
    Serial.printf("ESP-NOW: %s\n", espnow.isInitialized() ? "Initialized" : "Not initialized");
    Serial.printf("Mode: %s\n", isSenderMode ? "SENDER" : "RECEIVER");

    uint8_t mac[6];
    espnow.getMacAddress(mac);
    Serial.printf("This MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if (hasPeer) {
        Serial.printf("Peer MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                      peerMAC[0], peerMAC[1], peerMAC[2],
                      peerMAC[3], peerMAC[4], peerMAC[5]);
    } else {
        Serial.println(F("Peer MAC: None"));
    }

    Serial.printf("Packets Sent: %u\n", packetsSent);
    Serial.printf("Packets Received: %u\n", packetsReceived);
    Serial.println();
}

bool initESPNow() {
    if (espnow.isInitialized()) {
        Serial.println(F("ESP-NOW already initialized"));
        return true;
    }

    Serial.println(F("Initializing ESP-NOW via ESPNowHandler..."));

    // Use production ESPNowHandler
    if (!espnow.begin()) {
        Serial.println(F("ERROR: ESPNowHandler::begin() failed"));
        return false;
    }

    // Set callbacks using production interface
    espnow.setOnSendCallback(onSendCallback);
    espnow.setOnReceiveCallback(onReceiveCallback);

    Serial.println(F("ESP-NOW initialized successfully (using ESPNowHandler)!"));

    // Print MAC address
    uint8_t mac[6];
    espnow.getMacAddress(mac);
    Serial.printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return true;
}

void deinitESPNow() {
    if (!espnow.isInitialized()) {
        Serial.println(F("ESP-NOW not initialized"));
        return;
    }

    espnow.end();
    hasPeer = false;
    Serial.println(F("ESP-NOW deinitialized"));
}

bool parseMACAddress(const char* str, uint8_t* mac) {
    int values[6];
    if (sscanf(str, "%x:%x:%x:%x:%x:%x",
               &values[0], &values[1], &values[2],
               &values[3], &values[4], &values[5]) == 6) {
        for (int i = 0; i < 6; i++) {
            if (values[i] < 0 || values[i] > 255) {
                return false;
            }
            mac[i] = (uint8_t)values[i];
        }
        return true;
    }
    return false;
}

void addPeerInteractive() {
    if (!espnow.isInitialized()) {
        Serial.println(F("ERROR: Initialize ESP-NOW first (press 'i')"));
        return;
    }

    Serial.println(F("Enter peer MAC address (format: AA:BB:CC:DD:EE:FF):"));

    String input = "";
    unsigned long startTime = millis();
    const unsigned long TIMEOUT_MS = 30000;

    while (millis() - startTime < TIMEOUT_MS) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\r' || c == '\n') {
                if (input.length() > 0) {
                    Serial.println();
                    break;
                }
            } else if (c == 127 || c == 8) {  // Backspace
                if (input.length() > 0) {
                    input.remove(input.length() - 1);
                    Serial.write(8); Serial.write(' '); Serial.write(8);
                }
            } else {
                input += c;
                Serial.write(c);
            }
        }
        delay(10);
    }

    if (input.length() == 0) {
        Serial.println(F("Timeout - cancelled"));
        return;
    }

    uint8_t mac[6];
    if (!parseMACAddress(input.c_str(), mac)) {
        Serial.println(F("ERROR: Invalid MAC format. Use AA:BB:CC:DD:EE:FF"));
        return;
    }

    // Remove existing peer if any
    if (hasPeer) {
        espnow.removePeer(peerMAC);
    }

    // Add new peer using production ESPNowHandler
    if (!espnow.addPeer(mac)) {
        Serial.println(F("ERROR: ESPNowHandler::addPeer() failed"));
        return;
    }

    memcpy(peerMAC, mac, 6);
    hasPeer = true;

    Serial.printf("Peer added: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void addBroadcastPeer() {
    if (!espnow.isInitialized()) {
        Serial.println(F("ERROR: Initialize ESP-NOW first (press 'i')"));
        return;
    }

    uint8_t broadcastMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // Remove existing peer if any
    if (hasPeer) {
        espnow.removePeer(peerMAC);
    }

    // Add broadcast peer using production ESPNowHandler
    if (!espnow.addPeer(broadcastMAC)) {
        Serial.println(F("ERROR: ESPNowHandler::addPeer() failed for broadcast"));
        return;
    }

    memcpy(peerMAC, broadcastMAC, 6);
    hasPeer = true;

    Serial.println(F("Broadcast peer added: FF:FF:FF:FF:FF:FF"));
}

void clearPeer() {
    if (!espnow.isInitialized()) {
        Serial.println(F("ERROR: ESP-NOW not initialized"));
        return;
    }

    if (!hasPeer) {
        Serial.println(F("No peer to clear"));
        return;
    }

    espnow.removePeer(peerMAC);
    hasPeer = false;
    memset(peerMAC, 0, 6);
    Serial.println(F("Peer cleared"));
}

void sendTestPacket() {
    if (!espnow.isInitialized()) {
        Serial.println(F("ERROR: Initialize ESP-NOW first (press 'i')"));
        return;
    }

    if (!hasPeer) {
        Serial.println(F("ERROR: Add a peer first (press 'a' or 'b')"));
        return;
    }

    // Create simple test message
    const char* testMsg = "COW-Bois ESP-NOW Test";

    Serial.printf("Sending test packet to %02X:%02X:%02X:%02X:%02X:%02X...\n",
                  peerMAC[0], peerMAC[1], peerMAC[2],
                  peerMAC[3], peerMAC[4], peerMAC[5]);

    // Use production sendData()
    if (!espnow.sendData(peerMAC, (uint8_t*)testMsg, strlen(testMsg) + 1)) {
        Serial.println(F("ERROR: ESPNowHandler::sendData() failed"));
    }
}

void sendWeatherPacket() {
    if (!espnow.isInitialized()) {
        Serial.println(F("ERROR: Initialize ESP-NOW first (press 'i')"));
        return;
    }

    if (!hasPeer) {
        Serial.println(F("ERROR: Add a peer first (press 'a' or 'b')"));
        return;
    }

    // Create test WeatherReading with dummy data
    // This tests the production sendWeatherData() which packs the data
    WeatherReading reading;
    reading.timestamp = millis();
    reading.temperature = 23.5f;
    reading.humidity = 65.0f;
    reading.pressure = 1013.2f;
    reading.gasResistance = 150.0f;
    reading.windSpeed = 3.5f;
    reading.windDirection = 225;
    reading.precipitation = 0.0f;
    reading.lux = 45000;
    reading.solarIrradiance = 355.5f;
    reading.co2 = 420;
    reading.tvoc = 50;
    reading.isValid = true;

    Serial.printf("Sending weather packet to %02X:%02X:%02X:%02X:%02X:%02X...\n",
                  peerMAC[0], peerMAC[1], peerMAC[2],
                  peerMAC[3], peerMAC[4], peerMAC[5]);
    Serial.printf("Using ESPNowHandler::sendWeatherData() (packet size: %d bytes)\n",
                  sizeof(ESPNowPacket));

    // Use production sendWeatherData() - this tests packet packing
    if (!espnow.sendWeatherData(peerMAC, reading)) {
        Serial.println(F("ERROR: ESPNowHandler::sendWeatherData() failed"));
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
    Serial.println(F("   COW-Bois ESP-NOW Communication Test"));
    Serial.println(F("   (Using Production ESPNowHandler)"));
    Serial.println(F("========================================="));
    Serial.println();
    Serial.println(F("This test uses the actual ESPNowHandler class"));
    Serial.println(F("from src/communication/espnow_handler.cpp"));
    Serial.println();
    Serial.println(F("Testing requires TWO ESP32 devices:"));
    Serial.println(F("  1. Device A: Set as RECEIVER (press 'r')"));
    Serial.println(F("  2. Device B: Set as SENDER (press 's')"));
    Serial.println(F("  3. On Device B: Add Device A's MAC (press 'a')"));
    Serial.println(F("  4. Send packets from Device B (press 't' or 'w')"));
    Serial.println();

    // Auto-initialize ESP-NOW using production handler
    initESPNow();

    printHelp();
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();

        // Consume extra characters
        while (Serial.available()) {
            char c = Serial.read();
            if (c != '\r' && c != '\n') {
                break;
            }
        }

        switch (cmd) {
            case 'i':
            case 'I':
                initESPNow();
                break;

            case 'd':
            case 'D':
                deinitESPNow();
                break;

            case 'm':
            case 'M':
                {
                    uint8_t mac[6];
                    espnow.getMacAddress(mac);
                    Serial.printf("\nMAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n\n",
                                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                }
                break;

            case 's':
            case 'S':
                isSenderMode = true;
                Serial.println(F("\nMode set to: SENDER"));
                Serial.println(F("Add a peer (press 'a') then send packets (press 't' or 'w')\n"));
                break;

            case 'r':
            case 'R':
                isSenderMode = false;
                Serial.println(F("\nMode set to: RECEIVER"));
                Serial.println(F("Waiting for incoming packets...\n"));
                break;

            case 'a':
            case 'A':
                addPeerInteractive();
                break;

            case 'b':
            case 'B':
                addBroadcastPeer();
                break;

            case 'c':
            case 'C':
                clearPeer();
                break;

            case 't':
            case 'T':
                sendTestPacket();
                break;

            case 'w':
            case 'W':
                sendWeatherPacket();
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
}
