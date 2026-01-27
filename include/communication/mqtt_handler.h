/**
 * COW-Bois Weather Station - MQTT Handler
 * MQTT communication for main station data transmission
 */

#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "config.h"
#include "data/weather_data.h"

// MQTT message callback type
typedef void (*MQTTCallback)(const char* topic, const char* message);

class MQTTHandler {
public:
    MQTTHandler();

    /**
     * Initialize MQTT handler with broker details
     * @param broker Broker hostname or IP
     * @param port Broker port (default 1883)
     * @return true if connection successful
     */
    bool begin(const char* broker, uint16_t port = MQTT_PORT);

    /**
     * Connect to MQTT broker
     * @return true if connection successful
     */
    bool connect();

    /**
     * Disconnect from MQTT broker
     */
    void disconnect();

    /**
     * Check if connected to broker
     * @return true if connected
     */
    bool isConnected();

    /**
     * Process MQTT messages (call in loop)
     */
    void loop();

    /**
     * Publish raw string to topic
     * @param topic MQTT topic
     * @param payload Message payload
     * @param retained Whether to retain message
     * @return true if publish successful
     */
    bool publish(const char* topic, const char* payload, bool retained = false);

    /**
     * Publish weather data for a station
     * @param stationId Station identifier
     * @param reading Weather reading to publish
     * @return true if publish successful
     */
    bool publishWeatherData(const char* stationId, const WeatherReading& reading);

    /**
     * Publish station status
     * @param stationId Station identifier
     * @param status Status message
     * @return true if publish successful
     */
    bool publishStatus(const char* stationId, const char* status);

    /**
     * Subscribe to a topic
     * @param topic MQTT topic
     * @return true if subscription successful
     */
    bool subscribe(const char* topic);

    /**
     * Set message callback
     * @param callback Function to call when message received
     */
    void setCallback(MQTTCallback callback);

    /**
     * Set authentication credentials
     * @param username MQTT username
     * @param password MQTT password
     */
    void setCredentials(const char* username, const char* password);

    /**
     * Get PubSubClient state code
     * @return State code
     */
    int getState();

    /**
     * Get human-readable state string
     * @return State description
     */
    const char* getStateString();

private:
    WiFiClient _wifiClient;
    PubSubClient _client;
    bool _connected;

    char _broker[64];
    uint16_t _port;
    char _username[32];
    char _password[32];

    // Subscriptions tracking
    char _subscriptions[10][64];
    int _subscriptionCount;

    unsigned long _lastReconnectAttempt;

    // User callback
    MQTTCallback _messageCallback;

    /**
     * Handle incoming MQTT message
     */
    void handleCallback(char* topic, byte* payload, unsigned int length);

    /**
     * Format weather reading as JSON payload
     */
    void formatWeatherPayload(const WeatherReading& reading, char* buffer, size_t bufferSize);
};

#endif // MQTT_HANDLER_H
