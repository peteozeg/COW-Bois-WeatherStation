/**
 * COW-Bois Weather Station - ESP-NOW Handler
 * ESP-NOW communication for microstation to main station data transfer
 */

#ifndef ESPNOW_HANDLER_H
#define ESPNOW_HANDLER_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "config.h"
#include "data/weather_data.h"

// Callback types
typedef void (*ESPNowSendCallback)(const uint8_t* mac, bool success);
typedef void (*ESPNowReceiveCallback)(const uint8_t* mac, const uint8_t* data, int len);

class ESPNowHandler {
public:
    ESPNowHandler();

    /**
     * Initialize ESP-NOW
     * @return true if initialization successful
     */
    bool begin();

    /**
     * Deinitialize ESP-NOW
     */
    void end();

    /**
     * Add a peer device
     * @param macAddress MAC address of peer (6 bytes)
     * @param channel WiFi channel (0 = current)
     * @return true if peer added successfully
     */
    bool addPeer(const uint8_t* macAddress, uint8_t channel = 0);

    /**
     * Remove a peer device
     * @param macAddress MAC address of peer
     * @return true if peer removed successfully
     */
    bool removePeer(const uint8_t* macAddress);

    /**
     * Send raw data to peer
     * @param macAddress Destination MAC address
     * @param data Pointer to data buffer
     * @param length Data length (max 250 bytes)
     * @return true if send initiated
     */
    bool sendData(const uint8_t* macAddress, const uint8_t* data, size_t length);

    /**
     * Send weather data to peer
     * @param macAddress Destination MAC address
     * @param reading Weather reading to send
     * @return true if send initiated
     */
    bool sendWeatherData(const uint8_t* macAddress, const WeatherReading& reading);

    /**
     * Broadcast data to all peers
     * @param data Pointer to data buffer
     * @param length Data length
     * @return true if broadcast initiated
     */
    bool broadcast(const uint8_t* data, size_t length);

    /**
     * Parse received weather packet
     * @param data Received data buffer
     * @param length Data length
     * @param packet Output packet structure
     * @return true if parsing successful
     */
    bool parseWeatherPacket(const uint8_t* data, size_t length, ESPNowPacket& packet);

    /**
     * Set callback for send status
     * @param callback Function to call when send completes
     */
    void setOnSendCallback(ESPNowSendCallback callback);

    /**
     * Set callback for received data
     * @param callback Function to call when data received
     */
    void setOnReceiveCallback(ESPNowReceiveCallback callback);

    /**
     * Get this device's MAC address
     * @param mac Buffer to store MAC (6 bytes)
     */
    void getMacAddress(uint8_t* mac);

    /**
     * Get number of registered peers
     * @return Peer count
     */
    uint8_t getPeerCount() const { return _peerCount; }

    /**
     * Check if ESP-NOW is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return _initialized; }

private:
    bool _initialized;
    uint8_t _peerCount;

    // Peer storage
    esp_now_peer_info_t _peers[ESPNOW_MAX_PEERS];

    // User callbacks
    ESPNowSendCallback _sendCallback;
    ESPNowReceiveCallback _receiveCallback;

    // Static instance for callbacks
    static ESPNowHandler* _instance;

    // Static callbacks (required by ESP-NOW API)
    static void onSendStatic(const uint8_t* macAddress, esp_now_send_status_t status);
    static void onReceiveStatic(const uint8_t* macAddress, const uint8_t* data, int length);

    // Instance callback handlers
    void onSend(const uint8_t* macAddress, esp_now_send_status_t status);
    void onReceive(const uint8_t* macAddress, const uint8_t* data, int length);
};

#endif // ESPNOW_HANDLER_H
