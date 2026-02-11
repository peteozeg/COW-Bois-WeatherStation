/**
 * COW-Bois Weather Station - ESP-NOW Test
 *
 * Interactive test sketch to verify ESP-NOW communication between devices.
 * Supports both sender and receiver modes for testing microstation-to-main
 * station communication.
 *
 * Upload: pio run -e test_espnow -t upload
 * Monitor: pio device monitor
 *
 * Testing requires TWO devices:
 *   Device A: Receiver mode (press 'r')
 *   Device B: Sender mode (press 's'), add Device A's MAC (press 'a')
 */

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Include project headers
#include "config.h"
#include "data/weather_data.h"

// ============================================
// Forward Declarations
// ============================================
void printRawData(const uint8_t* data, int len);

// ============================================
// Global State
// ============================================
bool espnowInitialized = false;
bool isSenderMode = false;
uint8_t peerMAC[6] = {0};
bool hasPeer = false;
uint32_t packetsSent = 0;
uint32_t packetsReceived = 0;

// ============================================
// Callbacks
// ============================================
void onDataSent(const uint8_t* macAddr, esp_now_send_status_t status) {
    Serial.print(F("Send to "));
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X ",
                  macAddr[0], macAddr[1], macAddr[2],
                  macAddr[3], macAddr[4], macAddr[5]);
    if (status == ESP_NOW_SEND_SUCCESS) {
        Serial.println(F("SUCCESS"));
        packetsSent++;
    } else {
        Serial.println(F("FAILED"));
    }
}

void onDataReceived(const uint8_t* macAddr, const uint8_t* data, int len) {
    packetsReceived++;

    Serial.println();
    Serial.println(F("========== PACKET RECEIVED =========="));
    Serial.printf("From: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  macAddr[0], macAddr[1], macAddr[2],
                  macAddr[3], macAddr[4], macAddr[5]);
    Serial.printf("Length: %d bytes\n", len);

    // Check if it's a weather packet
    if (len == sizeof(ESPNowPacket)) {
        ESPNowPacket packet;
        memcpy(&packet, data, sizeof(ESPNowPacket));

        // Verify checksum
        uint8_t checksum = 0;
        for (size_t i = 0; i < sizeof(ESPNowPacket) - 1; i++) {
            checksum ^= data[i];
        }

        if (checksum == packet.checksum && packet.packetType == 0x01) {
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
            Serial.println(F("Type: Unknown/Invalid weather packet"));
            printRawData(data, len);
        }
    } else {
        Serial.println(F("Type: Raw Data"));
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
    Serial.println(F("  i - Initialize ESP-NOW"));
    Serial.println(F("  d - Deinitialize ESP-NOW"));
    Serial.println(F("  m - Show MAC address"));
    Serial.println(F("  s - Set SENDER mode"));
    Serial.println(F("  r - Set RECEIVER mode"));
    Serial.println(F("  a - Add peer (enter MAC address)"));
    Serial.println(F("  b - Add broadcast peer (FF:FF:FF:FF:FF:FF)"));
    Serial.println(F("  c - Clear peer"));
    Serial.println(F("  t - Send TEST packet (raw bytes)"));
    Serial.println(F("  w - Send WEATHER packet"));
    Serial.println(F("  x - Show statistics"));
    Serial.println(F("  h - Show this help"));
    Serial.println(F("============================================="));
    Serial.println();
}

void printStatus() {
    Serial.println();
    Serial.println(F("--- Current Status ---"));
    Serial.printf("ESP-NOW: %s\n", espnowInitialized ? "Initialized" : "Not initialized");
    Serial.printf("Mode: %s\n", isSenderMode ? "SENDER" : "RECEIVER");

    uint8_t mac[6];
    WiFi.macAddress(mac);
    Serial.printf("This MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if (hasPeer) {
        Serial.printf("Peer MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                      peerMAC[0], peerMAC[1], peerMAC[2],
                      peerMAC[3], peerMAC[4], peerMAC[5]);
    } else {
        Serial.println(F("Peer MAC: None"));
    }

    Serial.printf("Packets Sent: %lu\n", packetsSent);
    Serial.printf("Packets Received: %lu\n", packetsReceived);
    Serial.println();
}

bool initESPNow() {
    if (espnowInitialized) {
        Serial.println(F("ESP-NOW already initialized"));
        return true;
    }

    Serial.println(F("Initializing ESP-NOW..."));

    // Set WiFi mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println(F("ERROR: ESP-NOW init failed"));
        return false;
    }

    // Register callbacks
    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataReceived);

    espnowInitialized = true;

    // Print MAC address
    uint8_t mac[6];
    WiFi.macAddress(mac);
    Serial.println(F("ESP-NOW initialized successfully!"));
    Serial.printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return true;
}

void deinitESPNow() {
    if (!espnowInitialized) {
        Serial.println(F("ESP-NOW not initialized"));
        return;
    }

    esp_now_deinit();
    espnowInitialized = false;
    hasPeer = false;
    Serial.println(F("ESP-NOW deinitialized"));
}

bool parseMACAddress(const char* str, uint8_t* mac) {
    int values[6];
    if (sscanf(str, "%x:%x:%x:%x:%x:%x",
               &values[0], &values[1], &values[2],
               &values[3], &values[4], &values[5]) == 6) {
        // Validate each byte is in range 0-255
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
    if (!espnowInitialized) {
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
        esp_now_del_peer(peerMAC);
    }

    // Add new peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println(F("ERROR: Failed to add peer"));
        return;
    }

    memcpy(peerMAC, mac, 6);
    hasPeer = true;

    Serial.printf("Peer added: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void addBroadcastPeer() {
    if (!espnowInitialized) {
        Serial.println(F("ERROR: Initialize ESP-NOW first (press 'i')"));
        return;
    }

    uint8_t broadcastMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // Remove existing peer if any
    if (hasPeer) {
        esp_now_del_peer(peerMAC);
    }

    // Add broadcast peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println(F("ERROR: Failed to add broadcast peer"));
        return;
    }

    memcpy(peerMAC, broadcastMAC, 6);
    hasPeer = true;

    Serial.println(F("Broadcast peer added: FF:FF:FF:FF:FF:FF"));
}

void clearPeer() {
    if (!espnowInitialized) {
        Serial.println(F("ERROR: ESP-NOW not initialized"));
        return;
    }

    if (!hasPeer) {
        Serial.println(F("No peer to clear"));
        return;
    }

    esp_now_del_peer(peerMAC);
    hasPeer = false;
    memset(peerMAC, 0, 6);
    Serial.println(F("Peer cleared"));
}

void sendTestPacket() {
    if (!espnowInitialized) {
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

    esp_err_t result = esp_now_send(peerMAC, (uint8_t*)testMsg, strlen(testMsg) + 1);

    if (result != ESP_OK) {
        Serial.printf("ERROR: esp_now_send failed (%d)\n", result);
    }
}

void sendWeatherPacket() {
    if (!espnowInitialized) {
        Serial.println(F("ERROR: Initialize ESP-NOW first (press 'i')"));
        return;
    }

    if (!hasPeer) {
        Serial.println(F("ERROR: Add a peer first (press 'a' or 'b')"));
        return;
    }

    // Create test weather packet with dummy data
    ESPNowPacket packet;
    packet.packetType = 0x01;

    // Station ID from MAC
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(packet.stationId, sizeof(packet.stationId), "%02X%02X%02X%02X",
             mac[2], mac[3], mac[4], mac[5]);

    packet.timestamp = millis();
    packet.temperature = 2350;       // 23.50 C
    packet.humidity = 6500;          // 65.00 %
    packet.pressure = 10132;         // 1013.2 hPa
    packet.gasResistance = 1500;     // 150.0 KOhms
    packet.windSpeed = 350;          // 3.50 m/s
    packet.windDirection = 225;      // SW
    packet.precipitation = 0;        // 0 mm
    packet.lux = 45000;              // 45000 lux
    packet.co2 = 420;                // 420 ppm
    packet.tvoc = 50;                // 50 ppb
    packet.batteryVoltage = 3850;    // 3850 mV
    packet.flags = 0x01;             // Valid

    // Calculate checksum
    uint8_t* packetBytes = (uint8_t*)&packet;
    uint8_t checksum = 0;
    for (size_t i = 0; i < sizeof(ESPNowPacket) - 1; i++) {
        checksum ^= packetBytes[i];
    }
    packet.checksum = checksum;

    Serial.printf("Sending weather packet to %02X:%02X:%02X:%02X:%02X:%02X...\n",
                  peerMAC[0], peerMAC[1], peerMAC[2],
                  peerMAC[3], peerMAC[4], peerMAC[5]);
    Serial.printf("Packet size: %d bytes\n", sizeof(ESPNowPacket));

    esp_err_t result = esp_now_send(peerMAC, (uint8_t*)&packet, sizeof(ESPNowPacket));

    if (result != ESP_OK) {
        Serial.printf("ERROR: esp_now_send failed (%d)\n", result);
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
    Serial.println(F("========================================="));
    Serial.println();
    Serial.println(F("This test requires TWO ESP32 devices:"));
    Serial.println(F("  1. Device A: Set as RECEIVER (press 'r')"));
    Serial.println(F("  2. Device B: Set as SENDER (press 's')"));
    Serial.println(F("  3. On Device B: Add Device A's MAC (press 'a')"));
    Serial.println(F("  4. Send packets from Device B (press 't' or 'w')"));
    Serial.println();

    // Auto-initialize ESP-NOW
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
                // Put back if it's part of input (for MAC address)
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
                    WiFi.macAddress(mac);
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
