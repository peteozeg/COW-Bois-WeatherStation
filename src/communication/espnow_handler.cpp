/**
 * COW-Bois Weather Station - ESP-NOW Handler Implementation
 */

#include "communication/espnow_handler.h"
#include "config.h"
#include <WiFi.h>

// Static instance for callbacks
ESPNowHandler* ESPNowHandler::_instance = nullptr;

ESPNowHandler::ESPNowHandler()
    : _initialized(false)
    , _peerCount(0)
    , _sendCallback(nullptr)
    , _receiveCallback(nullptr) {
    _instance = this;
    memset(_peers, 0, sizeof(_peers));
}

bool ESPNowHandler::begin() {
    DEBUG_PRINTLN("ESP-NOW: Initializing...");

    // Set WiFi mode to Station
    WiFi.mode(WIFI_STA);

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        DEBUG_PRINTLN("ESP-NOW: Initialization failed");
        _initialized = false;
        return false;
    }

    // Register callbacks
    esp_now_register_send_cb(onSendStatic);
    esp_now_register_recv_cb(onReceiveStatic);

    _initialized = true;
    DEBUG_PRINTLN("ESP-NOW: Initialized successfully");

    // Print MAC address
    uint8_t mac[6];
    WiFi.macAddress(mac);
    DEBUG_PRINTF("ESP-NOW: MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return true;
}

void ESPNowHandler::end() {
    if (_initialized) {
        esp_now_deinit();
        _initialized = false;
        DEBUG_PRINTLN("ESP-NOW: Deinitialized");
    }
}

bool ESPNowHandler::addPeer(const uint8_t* macAddress, uint8_t channel) {
    if (!_initialized) return false;

    if (_peerCount >= ESPNOW_MAX_PEERS) {
        DEBUG_PRINTLN("ESP-NOW: Max peers reached");
        return false;
    }

    // Check if peer already exists
    for (int i = 0; i < _peerCount; i++) {
        if (memcmp(_peers[i].peer_addr, macAddress, 6) == 0) {
            DEBUG_PRINTLN("ESP-NOW: Peer already exists");
            return true;
        }
    }

    // Configure peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, macAddress, 6);
    peerInfo.channel = channel;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        DEBUG_PRINTLN("ESP-NOW: Failed to add peer");
        return false;
    }

    // Store peer info
    memcpy(&_peers[_peerCount], &peerInfo, sizeof(esp_now_peer_info_t));
    _peerCount++;

    DEBUG_PRINTF("ESP-NOW: Added peer %02X:%02X:%02X:%02X:%02X:%02X\n",
                 macAddress[0], macAddress[1], macAddress[2],
                 macAddress[3], macAddress[4], macAddress[5]);

    return true;
}

bool ESPNowHandler::removePeer(const uint8_t* macAddress) {
    if (!_initialized) return false;

    if (esp_now_del_peer(macAddress) != ESP_OK) {
        DEBUG_PRINTLN("ESP-NOW: Failed to remove peer");
        return false;
    }

    // Remove from local list
    for (int i = 0; i < _peerCount; i++) {
        if (memcmp(_peers[i].peer_addr, macAddress, 6) == 0) {
            // Shift remaining peers
            for (int j = i; j < _peerCount - 1; j++) {
                memcpy(&_peers[j], &_peers[j + 1], sizeof(esp_now_peer_info_t));
            }
            _peerCount--;
            break;
        }
    }

    DEBUG_PRINTLN("ESP-NOW: Peer removed");
    return true;
}

bool ESPNowHandler::sendData(const uint8_t* macAddress, const uint8_t* data, size_t length) {
    if (!_initialized) return false;

    if (length > ESPNOW_MAX_PACKET_SIZE) {
        DEBUG_PRINTLN("ESP-NOW: Data too large");
        return false;
    }

    esp_err_t result = esp_now_send(macAddress, data, length);

    if (result != ESP_OK) {
        DEBUG_PRINTF("ESP-NOW: Send failed with error %d\n", result);
        return false;
    }

    return true;
}

bool ESPNowHandler::sendWeatherData(const uint8_t* macAddress, const WeatherReading& reading) {
    if (!_initialized) return false;

    // Pack weather data into ESP-NOW packet
    ESPNowPacket packet;
    packet.packetType = 0x01;  // Weather data type

    // Copy station ID from WiFi MAC
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(packet.stationId, sizeof(packet.stationId), "%02X%02X%02X%02X",
             mac[2], mac[3], mac[4], mac[5]);

    packet.timestamp = reading.timestamp;
    packet.temperature = (int16_t)(reading.temperature * 100);  // 2 decimal places
    packet.humidity = (uint16_t)(reading.humidity * 100);
    packet.pressure = (uint16_t)(reading.pressure * 10);  // 1 decimal place
    packet.gasResistance = (uint16_t)(min(reading.gasResistance * 10.0f, 65535.0f));  // 1 decimal place, clamped
    packet.windSpeed = (uint16_t)(reading.windSpeed * 100);
    packet.windDirection = reading.windDirection;
    packet.precipitation = (uint16_t)(reading.precipitation * 100);
    packet.lux = reading.lux;
    packet.co2 = reading.co2;
    packet.tvoc = reading.tvoc;
    packet.batteryVoltage = 0;  // To be filled by power manager
    packet.flags = reading.isValid ? 0x01 : 0x00;

    // Calculate simple checksum
    uint8_t* packetBytes = (uint8_t*)&packet;
    uint8_t checksum = 0;
    for (size_t i = 0; i < sizeof(ESPNowPacket) - 1; i++) {
        checksum ^= packetBytes[i];
    }
    packet.checksum = checksum;

    return sendData(macAddress, (uint8_t*)&packet, sizeof(ESPNowPacket));
}

bool ESPNowHandler::broadcast(const uint8_t* data, size_t length) {
    // Broadcast address
    uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // Add broadcast peer if not already added
    if (!esp_now_is_peer_exist(broadcastAddr)) {
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, broadcastAddr, 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;
        esp_now_add_peer(&peerInfo);
    }

    return sendData(broadcastAddr, data, length);
}

void ESPNowHandler::onSendStatic(const uint8_t* macAddress, esp_now_send_status_t status) {
    if (_instance) {
        _instance->onSend(macAddress, status);
    }
}

void ESPNowHandler::onReceiveStatic(const uint8_t* macAddress, const uint8_t* data, int length) {
    if (_instance) {
        _instance->onReceive(macAddress, data, length);
    }
}

void ESPNowHandler::onSend(const uint8_t* macAddress, esp_now_send_status_t status) {
    DEBUG_PRINTF("ESP-NOW: Send to %02X:%02X:%02X:%02X:%02X:%02X %s\n",
                 macAddress[0], macAddress[1], macAddress[2],
                 macAddress[3], macAddress[4], macAddress[5],
                 status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAILED");

    if (_sendCallback) {
        _sendCallback(macAddress, status == ESP_NOW_SEND_SUCCESS);
    }
}

void ESPNowHandler::onReceive(const uint8_t* macAddress, const uint8_t* data, int length) {
    DEBUG_PRINTF("ESP-NOW: Received %d bytes from %02X:%02X:%02X:%02X:%02X:%02X\n",
                 length,
                 macAddress[0], macAddress[1], macAddress[2],
                 macAddress[3], macAddress[4], macAddress[5]);

    if (_receiveCallback) {
        _receiveCallback(macAddress, data, length);
    }
}

void ESPNowHandler::setOnSendCallback(ESPNowSendCallback callback) {
    _sendCallback = callback;
}

void ESPNowHandler::setOnReceiveCallback(ESPNowReceiveCallback callback) {
    _receiveCallback = callback;
}

bool ESPNowHandler::parseWeatherPacket(const uint8_t* data, size_t length, ESPNowPacket& packet) {
    if (length != sizeof(ESPNowPacket)) {
        DEBUG_PRINTLN("ESP-NOW: Invalid packet size");
        return false;
    }

    memcpy(&packet, data, sizeof(ESPNowPacket));

    // Verify checksum
    uint8_t checksum = 0;
    for (size_t i = 0; i < sizeof(ESPNowPacket) - 1; i++) {
        checksum ^= data[i];
    }

    if (checksum != packet.checksum) {
        DEBUG_PRINTLN("ESP-NOW: Checksum mismatch");
        return false;
    }

    return true;
}

void ESPNowHandler::getMacAddress(uint8_t* mac) {
    WiFi.macAddress(mac);
}

