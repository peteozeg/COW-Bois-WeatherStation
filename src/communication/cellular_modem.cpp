/**
 * COW-Bois Weather Station - Cellular Modem Implementation
 * SIM7600 4G LTE modem interface
 */

#include "communication/cellular_modem.h"
#include "config.h"

CellularModem::CellularModem()
    : _modemSerial(nullptr)
    , _initialized(false)
    , _connected(false)
    , _signalQuality(0) {
    memset(_imei, 0, sizeof(_imei));
    memset(_operatorName, 0, sizeof(_operatorName));
}

bool CellularModem::begin(HardwareSerial& serial, uint8_t rxPin, uint8_t txPin,
                          uint8_t powerPin, uint8_t resetPin) {
    DEBUG_PRINTLN("Modem: Initializing SIM7600...");

    _modemSerial = &serial;
    _powerPin = powerPin;
    _resetPin = resetPin;

    // Configure control pins
    if (_powerPin != 255) {
        pinMode(_powerPin, OUTPUT);
        digitalWrite(_powerPin, LOW);
    }
    if (_resetPin != 255) {
        pinMode(_resetPin, OUTPUT);
        digitalWrite(_resetPin, HIGH);
    }

    // Start serial communication
    _modemSerial->begin(MODEM_BAUD_RATE, SERIAL_8N1, rxPin, txPin);

    // Power on modem
    powerOn();

    // Wait for modem to be ready
    delay(3000);

    // Test communication
    if (!sendATCommand("AT", "OK", 1000)) {
        DEBUG_PRINTLN("Modem: No response to AT command");
        _initialized = false;
        return false;
    }

    // Disable echo
    sendATCommand("ATE0", "OK", 1000);

    // Get modem info
    if (sendATCommand("AT+CGSN", "OK", 1000)) {
        // Parse IMEI from response
        parseIMEI();
    }

    // Check SIM card
    if (!sendATCommand("AT+CPIN?", "READY", 5000)) {
        DEBUG_PRINTLN("Modem: SIM card not ready");
        _initialized = false;
        return false;
    }

    _initialized = true;
    DEBUG_PRINTLN("Modem: Initialized successfully");
    DEBUG_PRINTF("Modem: IMEI: %s\n", _imei);

    return true;
}

void CellularModem::powerOn() {
    DEBUG_PRINTLN("Modem: Powering on...");

    if (_powerPin != 255) {
        digitalWrite(_powerPin, HIGH);
        delay(1000);
        digitalWrite(_powerPin, LOW);
        delay(2000);
    }
}

void CellularModem::powerOff() {
    DEBUG_PRINTLN("Modem: Powering off...");

    sendATCommand("AT+CPOF", "OK", 5000);

    if (_powerPin != 255) {
        digitalWrite(_powerPin, HIGH);
        delay(3000);
        digitalWrite(_powerPin, LOW);
    }

    _connected = false;
}

void CellularModem::reset() {
    DEBUG_PRINTLN("Modem: Resetting...");

    if (_resetPin != 255) {
        digitalWrite(_resetPin, LOW);
        delay(500);
        digitalWrite(_resetPin, HIGH);
        delay(3000);
    }

    _connected = false;
}

bool CellularModem::connect(const char* apn, const char* user, const char* pass) {
    if (!_initialized) return false;

    DEBUG_PRINTF("Modem: Connecting with APN: %s\n", apn);

    // Set APN
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CGDCONT=1,\"IP\",\"%s\"", apn);
    if (!sendATCommand(cmd, "OK", 5000)) {
        DEBUG_PRINTLN("Modem: Failed to set APN");
        return false;
    }

    // Activate PDP context
    if (!sendATCommand("AT+CGACT=1,1", "OK", 30000)) {
        DEBUG_PRINTLN("Modem: Failed to activate PDP context");
        return false;
    }

    // Check registration
    for (int i = 0; i < 30; i++) {
        if (sendATCommand("AT+CREG?", "+CREG: 0,1", 1000) ||
            sendATCommand("AT+CREG?", "+CREG: 0,5", 1000)) {
            _connected = true;
            break;
        }
        delay(1000);
    }

    if (_connected) {
        DEBUG_PRINTLN("Modem: Connected to network");
        updateSignalQuality();
        getOperator();
    } else {
        DEBUG_PRINTLN("Modem: Failed to register on network");
    }

    return _connected;
}

void CellularModem::disconnect() {
    sendATCommand("AT+CGACT=0,1", "OK", 5000);
    _connected = false;
    DEBUG_PRINTLN("Modem: Disconnected");
}

bool CellularModem::isConnected() {
    if (!_initialized) return false;

    // Quick check with registration status
    if (sendATCommand("AT+CREG?", "+CREG: 0,1", 1000) ||
        sendATCommand("AT+CREG?", "+CREG: 0,5", 1000)) {
        _connected = true;
    } else {
        _connected = false;
    }

    return _connected;
}

int CellularModem::getSignalQuality() {
    updateSignalQuality();
    return _signalQuality;
}

bool CellularModem::sendHTTPPost(const char* url, const char* data, char* response, size_t responseSize) {
    if (!_connected) return false;

    DEBUG_PRINTF("Modem: HTTP POST to %s\n", url);

    // Initialize HTTP service
    if (!sendATCommand("AT+HTTPINIT", "OK", 5000)) {
        sendATCommand("AT+HTTPTERM", "OK", 1000);
        if (!sendATCommand("AT+HTTPINIT", "OK", 5000)) {
            return false;
        }
    }

    // Set URL
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "AT+HTTPPARA=\"URL\",\"%s\"", url);
    if (!sendATCommand(cmd, "OK", 5000)) {
        sendATCommand("AT+HTTPTERM", "OK", 1000);
        return false;
    }

    // Set content type
    sendATCommand("AT+HTTPPARA=\"CONTENT\",\"application/json\"", "OK", 1000);

    // Send data
    size_t dataLen = strlen(data);
    snprintf(cmd, sizeof(cmd), "AT+HTTPDATA=%d,10000", dataLen);
    if (!sendATCommand(cmd, "DOWNLOAD", 5000)) {
        sendATCommand("AT+HTTPTERM", "OK", 1000);
        return false;
    }

    // Send the actual data
    _modemSerial->print(data);
    delay(1000);

    // Execute POST
    if (!sendATCommand("AT+HTTPACTION=1", "+HTTPACTION:", 30000)) {
        sendATCommand("AT+HTTPTERM", "OK", 1000);
        return false;
    }

    // Read response
    if (sendATCommand("AT+HTTPREAD", "+HTTPREAD:", 5000)) {
        // Parse response from buffer
        if (response && responseSize > 0) {
            strncpy(response, _responseBuffer, responseSize - 1);
            response[responseSize - 1] = '\0';
        }
    }

    // Terminate HTTP session
    sendATCommand("AT+HTTPTERM", "OK", 1000);

    DEBUG_PRINTLN("Modem: HTTP POST complete");
    return true;
}

bool CellularModem::sendHTTPGet(const char* url, char* response, size_t responseSize) {
    if (!_connected) return false;

    DEBUG_PRINTF("Modem: HTTP GET from %s\n", url);

    // Initialize HTTP service
    if (!sendATCommand("AT+HTTPINIT", "OK", 5000)) {
        sendATCommand("AT+HTTPTERM", "OK", 1000);
        if (!sendATCommand("AT+HTTPINIT", "OK", 5000)) {
            return false;
        }
    }

    // Set URL
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "AT+HTTPPARA=\"URL\",\"%s\"", url);
    if (!sendATCommand(cmd, "OK", 5000)) {
        sendATCommand("AT+HTTPTERM", "OK", 1000);
        return false;
    }

    // Execute GET
    if (!sendATCommand("AT+HTTPACTION=0", "+HTTPACTION:", 30000)) {
        sendATCommand("AT+HTTPTERM", "OK", 1000);
        return false;
    }

    // Read response
    if (sendATCommand("AT+HTTPREAD", "+HTTPREAD:", 5000)) {
        if (response && responseSize > 0) {
            strncpy(response, _responseBuffer, responseSize - 1);
            response[responseSize - 1] = '\0';
        }
    }

    // Terminate HTTP session
    sendATCommand("AT+HTTPTERM", "OK", 1000);

    return true;
}

bool CellularModem::sendSMS(const char* phoneNumber, const char* message) {
    if (!_initialized) return false;

    DEBUG_PRINTF("Modem: Sending SMS to %s\n", phoneNumber);

    // Set text mode
    sendATCommand("AT+CMGF=1", "OK", 1000);

    // Start message
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+CMGS=\"%s\"", phoneNumber);
    if (!sendATCommand(cmd, ">", 5000)) {
        return false;
    }

    // Send message content
    _modemSerial->print(message);
    _modemSerial->write(0x1A);  // Ctrl+Z to send

    // Wait for confirmation
    return waitForResponse("+CMGS:", 30000);
}

bool CellularModem::sendATCommand(const char* cmd, const char* expectedResponse, uint32_t timeout) {
    if (!_modemSerial) return false;

    // Clear any pending data
    while (_modemSerial->available()) {
        _modemSerial->read();
    }

    // Send command
    _modemSerial->println(cmd);
    DEBUG_PRINTF("Modem TX: %s\n", cmd);

    // Wait for response
    return waitForResponse(expectedResponse, timeout);
}

bool CellularModem::waitForResponse(const char* expected, uint32_t timeout) {
    if (!_modemSerial) return false;

    unsigned long start = millis();
    int bufferIndex = 0;
    memset(_responseBuffer, 0, sizeof(_responseBuffer));

    while ((millis() - start) < timeout) {
        while (_modemSerial->available()) {
            char c = _modemSerial->read();
            if (bufferIndex < (int)sizeof(_responseBuffer) - 1) {
                _responseBuffer[bufferIndex++] = c;
            }

            // Check if we have the expected response
            if (strstr(_responseBuffer, expected) != nullptr) {
                DEBUG_PRINTF("Modem RX: %s\n", _responseBuffer);
                return true;
            }

            // Check for error
            if (strstr(_responseBuffer, "ERROR") != nullptr) {
                DEBUG_PRINTF("Modem RX (ERROR): %s\n", _responseBuffer);
                return false;
            }
        }
        delay(10);
    }

    DEBUG_PRINTF("Modem RX (TIMEOUT): %s\n", _responseBuffer);
    return false;
}

void CellularModem::updateSignalQuality() {
    if (sendATCommand("AT+CSQ", "+CSQ:", 2000)) {
        // Parse signal quality from response
        // Format: +CSQ: <rssi>,<ber>
        char* ptr = strstr(_responseBuffer, "+CSQ:");
        if (ptr) {
            int rssi;
            if (sscanf(ptr, "+CSQ: %d,", &rssi) == 1) {
                // Convert RSSI to dBm (roughly)
                // 0: -113 dBm or less
                // 1: -111 dBm
                // 2-30: -109 to -53 dBm
                // 31: -51 dBm or greater
                // 99: not known or not detectable
                if (rssi == 99) {
                    _signalQuality = 0;
                } else {
                    _signalQuality = -113 + (rssi * 2);
                }
            }
        }
    }
}

void CellularModem::getOperator() {
    if (sendATCommand("AT+COPS?", "+COPS:", 2000)) {
        char* ptr = strstr(_responseBuffer, "\"");
        if (ptr) {
            ptr++;
            char* end = strchr(ptr, '"');
            if (end) {
                size_t len = end - ptr;
                if (len < sizeof(_operatorName)) {
                    strncpy(_operatorName, ptr, len);
                    _operatorName[len] = '\0';
                }
            }
        }
    }
    DEBUG_PRINTF("Modem: Operator: %s\n", _operatorName);
}

void CellularModem::parseIMEI() {
    // IMEI is typically in the response after AT+CGSN
    // Look for a 15-digit number
    char* ptr = _responseBuffer;
    while (*ptr) {
        if (*ptr >= '0' && *ptr <= '9') {
            int len = 0;
            while (ptr[len] >= '0' && ptr[len] <= '9' && len < 15) {
                len++;
            }
            if (len == 15) {
                strncpy(_imei, ptr, 15);
                _imei[15] = '\0';
                return;
            }
        }
        ptr++;
    }
}

void CellularModem::sleep() {
    if (!_initialized) return;
    DEBUG_PRINTLN("Modem: Entering sleep mode");
    sendATCommand("AT+CSCLK=2", "OK", 1000);
}

void CellularModem::wake() {
    if (!_initialized || !_modemSerial) return;
    DEBUG_PRINTLN("Modem: Waking up");
    // Send any character to wake up
    _modemSerial->println("AT");
    delay(100);
    sendATCommand("AT+CSCLK=0", "OK", 1000);
}

