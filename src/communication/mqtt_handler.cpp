/**
 * COW-Bois Weather Station - MQTT Handler Implementation
 */

#include "communication/mqtt_handler.h"
#include "config.h"
#include <WiFi.h>

MQTTHandler::MQTTHandler()
    : _client(_wifiClient)
    , _connected(false)
    , _port(MQTT_PORT)
    , _subscriptionCount(0)
    , _lastReconnectAttempt(0)
    , _messageCallback(nullptr) {
    memset(_broker, 0, sizeof(_broker));
    memset(_username, 0, sizeof(_username));
    memset(_password, 0, sizeof(_password));
}

bool MQTTHandler::begin(const char* broker, uint16_t port) {
    DEBUG_PRINTF("MQTT: Connecting to broker %s:%d\n", broker, port);

    strncpy(_broker, broker, sizeof(_broker) - 1);
    _port = port;

    _client.setServer(_broker, _port);
    _client.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->handleCallback(topic, payload, length);
    });

    // Set buffer size for larger messages
    _client.setBufferSize(MQTT_MAX_PACKET_SIZE);

    return connect();
}

bool MQTTHandler::connect() {
    if (_client.connected()) {
        _connected = true;
        return true;
    }

    DEBUG_PRINTLN("MQTT: Attempting connection...");

    // Generate client ID from MAC address
    char clientId[32];
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(clientId, sizeof(clientId), "cowbois-%02X%02X%02X",
             mac[3], mac[4], mac[5]);

    bool success;
    if (strlen(_username) > 0) {
        success = _client.connect(clientId, _username, _password);
    } else {
        success = _client.connect(clientId);
    }

    if (success) {
        _connected = true;
        DEBUG_PRINTLN("MQTT: Connected successfully");

        // Resubscribe to topics
        for (int i = 0; i < _subscriptionCount; i++) {
            _client.subscribe(_subscriptions[i]);
            DEBUG_PRINTF("MQTT: Resubscribed to %s\n", _subscriptions[i]);
        }
    } else {
        _connected = false;
        DEBUG_PRINTF("MQTT: Connection failed, rc=%d\n", _client.state());
    }

    return _connected;
}

void MQTTHandler::disconnect() {
    if (_client.connected()) {
        _client.disconnect();
    }
    _connected = false;
    DEBUG_PRINTLN("MQTT: Disconnected");
}

bool MQTTHandler::isConnected() {
    _connected = _client.connected();
    return _connected;
}

void MQTTHandler::loop() {
    if (!_client.connected()) {
        _connected = false;

        // Attempt reconnection with backoff
        unsigned long now = millis();
        if (now - _lastReconnectAttempt > MQTT_RECONNECT_INTERVAL) {
            _lastReconnectAttempt = now;
            if (connect()) {
                _lastReconnectAttempt = 0;
            }
        }
    } else {
        _client.loop();
    }
}

bool MQTTHandler::publish(const char* topic, const char* payload, bool retained) {
    if (!_client.connected()) {
        DEBUG_PRINTLN("MQTT: Cannot publish - not connected");
        return false;
    }

    bool success = _client.publish(topic, payload, retained);
    if (success) {
        DEBUG_PRINTF("MQTT: Published to %s\n", topic);
    } else {
        DEBUG_PRINTF("MQTT: Failed to publish to %s\n", topic);
    }

    return success;
}

bool MQTTHandler::publishWeatherData(const char* stationId, const WeatherReading& reading) {
    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%s/weather", MQTT_TOPIC_PREFIX, stationId);

    char payload[512];
    formatWeatherPayload(reading, payload, sizeof(payload));

    return publish(topic, payload, false);
}

bool MQTTHandler::publishStatus(const char* stationId, const char* status) {
    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%s/status", MQTT_TOPIC_PREFIX, stationId);

    return publish(topic, status, true);  // Retain status messages
}

bool MQTTHandler::subscribe(const char* topic) {
    if (_subscriptionCount >= 10) {
        DEBUG_PRINTLN("MQTT: Max subscriptions reached");
        return false;
    }

    strncpy(_subscriptions[_subscriptionCount], topic, sizeof(_subscriptions[0]) - 1);
    _subscriptionCount++;

    if (_client.connected()) {
        return _client.subscribe(topic);
    }

    return true;  // Will subscribe on connect
}

void MQTTHandler::setCallback(MQTTCallback callback) {
    _messageCallback = callback;
}

void MQTTHandler::setCredentials(const char* username, const char* password) {
    strncpy(_username, username, sizeof(_username) - 1);
    strncpy(_password, password, sizeof(_password) - 1);
}

void MQTTHandler::handleCallback(char* topic, byte* payload, unsigned int length) {
    // Null-terminate the payload
    char message[256];
    size_t copyLen = length < sizeof(message) - 1 ? length : sizeof(message) - 1;
    memcpy(message, payload, copyLen);
    message[copyLen] = '\0';

    DEBUG_PRINTF("MQTT: Received on %s: %s\n", topic, message);

    if (_messageCallback) {
        _messageCallback(topic, message);
    }
}

void MQTTHandler::formatWeatherPayload(const WeatherReading& reading, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize,
        "{"
        "\"timestamp\":%lu,"
        "\"temperature\":%.2f,"
        "\"humidity\":%.2f,"
        "\"pressure\":%.2f,"
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
        reading.windSpeed,
        reading.windDirection,
        reading.precipitation,
        reading.lux,
        reading.solarIrradiance,
        reading.co2,
        reading.tvoc,
        reading.isValid ? "true" : "false"
    );
}

int MQTTHandler::getState() {
    return _client.state();
}

const char* MQTTHandler::getStateString() {
    switch (_client.state()) {
        case -4: return "CONNECTION_TIMEOUT";
        case -3: return "CONNECTION_LOST";
        case -2: return "CONNECT_FAILED";
        case -1: return "DISCONNECTED";
        case 0:  return "CONNECTED";
        case 1:  return "CONNECT_BAD_PROTOCOL";
        case 2:  return "CONNECT_BAD_CLIENT_ID";
        case 3:  return "CONNECT_UNAVAILABLE";
        case 4:  return "CONNECT_BAD_CREDENTIALS";
        case 5:  return "CONNECT_UNAUTHORIZED";
        default: return "UNKNOWN";
    }
}

