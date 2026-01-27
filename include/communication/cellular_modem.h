/**
 * COW-Bois Weather Station - Cellular Modem
 * SIM7600 4G LTE modem interface for LILYGO T-SIM7600G-H
 */

#ifndef CELLULAR_MODEM_H
#define CELLULAR_MODEM_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "pin_definitions.h"
#include "config.h"

class CellularModem {
public:
    CellularModem();

    /**
     * Initialize modem with serial interface
     * @param serial HardwareSerial instance
     * @param rxPin RX pin
     * @param txPin TX pin
     * @param powerPin Power control pin
     * @param resetPin Reset pin
     * @return true if initialization successful
     */
    bool begin(HardwareSerial& serial, uint8_t rxPin, uint8_t txPin,
               uint8_t powerPin = 255, uint8_t resetPin = 255);

    /**
     * Power on the modem
     */
    void powerOn();

    /**
     * Power off the modem
     */
    void powerOff();

    /**
     * Reset the modem
     */
    void reset();

    /**
     * Connect to cellular network
     * @param apn Access Point Name
     * @param user Username (optional)
     * @param pass Password (optional)
     * @return true if connection successful
     */
    bool connect(const char* apn, const char* user = "", const char* pass = "");

    /**
     * Disconnect from network
     */
    void disconnect();

    /**
     * Check if connected to network
     * @return true if data connection active
     */
    bool isConnected();

    /**
     * Get signal quality in dBm
     * @return Signal strength
     */
    int getSignalQuality();

    /**
     * Send HTTP POST request
     * @param url Target URL
     * @param data POST data
     * @param response Response buffer
     * @param responseSize Response buffer size
     * @return true if successful
     */
    bool sendHTTPPost(const char* url, const char* data,
                      char* response = nullptr, size_t responseSize = 0);

    /**
     * Send HTTP GET request
     * @param url Target URL
     * @param response Response buffer
     * @param responseSize Response buffer size
     * @return true if successful
     */
    bool sendHTTPGet(const char* url, char* response = nullptr, size_t responseSize = 0);

    /**
     * Send SMS message
     * @param phoneNumber Destination phone number
     * @param message Message text
     * @return true if sent successfully
     */
    bool sendSMS(const char* phoneNumber, const char* message);

    /**
     * Put modem in low power mode
     */
    void sleep();

    /**
     * Wake modem from sleep
     */
    void wake();

    /**
     * Get IMEI number
     * @return IMEI string
     */
    const char* getIMEI() const { return _imei; }

    /**
     * Get network operator name
     * @return Operator name string
     */
    const char* getOperatorName() const { return _operatorName; }

    /**
     * Check if modem is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return _initialized; }

private:
    HardwareSerial* _modemSerial;
    bool _initialized;
    bool _connected;
    int _signalQuality;

    uint8_t _powerPin;
    uint8_t _resetPin;

    char _imei[20];
    char _operatorName[32];
    char _responseBuffer[512];

    /**
     * Send AT command and wait for expected response
     * @param cmd AT command to send
     * @param expectedResponse Expected response string
     * @param timeout Timeout in ms
     * @return true if expected response received
     */
    bool sendATCommand(const char* cmd, const char* expectedResponse, uint32_t timeout = 1000);

    /**
     * Wait for specific response
     * @param expected Expected response string
     * @param timeout Timeout in ms
     * @return true if response received
     */
    bool waitForResponse(const char* expected, uint32_t timeout);

    /**
     * Update signal quality reading
     */
    void updateSignalQuality();

    /**
     * Get and store operator name
     */
    void getOperator();

    /**
     * Parse IMEI from response
     */
    void parseIMEI();
};

#endif // CELLULAR_MODEM_H
