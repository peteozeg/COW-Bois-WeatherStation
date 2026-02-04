/**
 * COW-Bois Weather Station - Cellular Modem Test
 *
 * Simple test sketch to verify SIM7600 modem communication.
 * Tests basic AT commands, SIM status, signal quality, and network registration.
 *
 * Upload: pio run -e test_modem -t upload
 * Monitor: pio device monitor
 *
 * Wiring (from pin_definitions.h):
 *   MODEM_TX  -> GPIO 27
 *   MODEM_RX  -> GPIO 26
 *   PWRKEY    -> GPIO 4
 *   RESET     -> GPIO 5
 *   POWER     -> GPIO 23
 */

#include <Arduino.h>

// Modem pins (from pin_definitions.h)
#define MODEM_TX_PIN    27
#define MODEM_RX_PIN    26
#define MODEM_PWRKEY    4
#define MODEM_RESET     5
// Note: MODEM_POWER (GPIO 23) may not be connected on all boards
// Set to 255 to disable if your board doesn't have a power enable pin
#define MODEM_POWER     23   // Set to 255 if not used

// Use HardwareSerial for modem communication
HardwareSerial ModemSerial(1);  // UART1

// Buffer for responses
char responseBuffer[512];

// ============================================
// Helper Functions
// ============================================

void printHelp() {
    Serial.println();
    Serial.println(F("=== Modem Test Commands ==="));
    Serial.println(F("  a - Test AT communication"));
    Serial.println(F("  i - Get modem info (IMEI)"));
    Serial.println(F("  p - Check SIM card (PIN status)"));
    Serial.println(F("  s - Check signal strength"));
    Serial.println(F("  n - Check network registration"));
    Serial.println(F("  o - Get operator name"));
    Serial.println(F("  r - Reset modem"));
    Serial.println(F("  w - Power cycle modem"));
    Serial.println(F("  m - Manual AT command mode (type 'exit' to quit)"));
    Serial.println(F("  h - Show this help"));
    Serial.println();
}

// Send AT command and wait for response
bool sendATCommand(const char* command, const char* expected, unsigned long timeout = 2000) {
    // Clear any pending data
    while (ModemSerial.available()) {
        ModemSerial.read();
    }

    Serial.print(F(">> "));
    Serial.println(command);

    ModemSerial.println(command);

    unsigned long start = millis();
    size_t index = 0;
    memset(responseBuffer, 0, sizeof(responseBuffer));

    while (millis() - start < timeout) {
        while (ModemSerial.available()) {
            char c = ModemSerial.read();
            if (index < sizeof(responseBuffer) - 1) {
                responseBuffer[index++] = c;
            }
        }

        // Check if we got the expected response
        if (expected && strstr(responseBuffer, expected)) {
            break;
        }

        // Also check for ERROR
        if (strstr(responseBuffer, "ERROR")) {
            break;
        }

        delay(10);
    }

    // Print response
    if (index > 0) {
        Serial.print(F("<< "));
        Serial.println(responseBuffer);
    }

    return (expected && strstr(responseBuffer, expected));
}

void powerOnModem() {
    Serial.println(F("Powering on modem..."));

    // Enable power supply (if pin is configured)
    #if MODEM_POWER != 255
    digitalWrite(MODEM_POWER, HIGH);
    delay(100);
    #endif

    // Pulse PWRKEY to turn on (hold for 1 second)
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(1000);
    digitalWrite(MODEM_PWRKEY, LOW);

    Serial.println(F("Waiting for modem to boot (5 seconds)..."));
    delay(5000);
}

void resetModem() {
    Serial.println(F("Resetting modem..."));

    digitalWrite(MODEM_RESET, LOW);
    delay(500);
    digitalWrite(MODEM_RESET, HIGH);

    Serial.println(F("Waiting for modem to restart (5 seconds)..."));
    delay(5000);
}

void powerCycleModem() {
    Serial.println(F("Power cycling modem..."));

    // Power off via PWRKEY (same as power on - it's a toggle)
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(1000);
    digitalWrite(MODEM_PWRKEY, LOW);
    delay(2000);

    // Also cut power supply if pin is configured
    #if MODEM_POWER != 255
    digitalWrite(MODEM_POWER, LOW);
    delay(1000);
    #endif

    // Power on
    powerOnModem();
}

void testATCommunication() {
    Serial.println(F("\n--- Testing AT Communication ---"));

    if (sendATCommand("AT", "OK")) {
        Serial.println(F("SUCCESS: Modem is responding!"));
    } else {
        Serial.println(F("FAILED: No response from modem"));
        Serial.println(F("Try: 'w' to power cycle, or check wiring"));
    }
}

void getModemInfo() {
    Serial.println(F("\n--- Modem Information ---"));

    // Disable echo first
    sendATCommand("ATE0", "OK");

    // Get IMEI
    Serial.println(F("IMEI:"));
    sendATCommand("AT+CGSN", "OK");

    // Get firmware version
    Serial.println(F("\nFirmware:"));
    sendATCommand("AT+CGMR", "OK");

    // Get model
    Serial.println(F("\nModel:"));
    sendATCommand("AT+CGMM", "OK");
}

void checkSIMStatus() {
    Serial.println(F("\n--- SIM Card Status ---"));

    if (sendATCommand("AT+CPIN?", "READY")) {
        Serial.println(F("SIM card is READY"));
    } else if (strstr(responseBuffer, "SIM PIN")) {
        Serial.println(F("SIM requires PIN code"));
    } else if (strstr(responseBuffer, "SIM PUK")) {
        Serial.println(F("SIM is PUK locked!"));
    } else if (strstr(responseBuffer, "NOT INSERTED")) {
        Serial.println(F("No SIM card detected!"));
    } else {
        Serial.println(F("Unknown SIM status"));
    }
}

void checkSignalStrength() {
    Serial.println(F("\n--- Signal Strength ---"));

    sendATCommand("AT+CSQ", "+CSQ:");

    // Parse CSQ value
    char* csqPtr = strstr(responseBuffer, "+CSQ:");
    if (csqPtr) {
        int rssi = atoi(csqPtr + 6);

        Serial.print(F("RSSI: "));
        Serial.print(rssi);

        if (rssi == 99) {
            Serial.println(F(" (Unknown/No signal)"));
        } else {
            // Convert to dBm: dBm = -113 + (rssi * 2)
            int dbm = -113 + (rssi * 2);
            Serial.print(F(" ("));
            Serial.print(dbm);
            Serial.print(F(" dBm) - "));

            if (rssi < 10) {
                Serial.println(F("Poor"));
            } else if (rssi < 15) {
                Serial.println(F("Fair"));
            } else if (rssi < 20) {
                Serial.println(F("Good"));
            } else {
                Serial.println(F("Excellent"));
            }
        }
    } else {
        Serial.println(F("Failed to get signal strength"));
    }
}

void checkNetworkRegistration() {
    Serial.println(F("\n--- Network Registration ---"));

    sendATCommand("AT+CREG?", "+CREG:");

    char* cregPtr = strstr(responseBuffer, "+CREG:");
    if (cregPtr) {
        // Format: +CREG: <n>,<stat>
        char* statPtr = strchr(cregPtr + 7, ',');
        if (statPtr) {
            int stat = atoi(statPtr + 1);

            Serial.print(F("Status: "));
            switch (stat) {
                case 0: Serial.println(F("Not registered, not searching")); break;
                case 1: Serial.println(F("Registered, home network")); break;
                case 2: Serial.println(F("Not registered, searching...")); break;
                case 3: Serial.println(F("Registration denied")); break;
                case 4: Serial.println(F("Unknown")); break;
                case 5: Serial.println(F("Registered, roaming")); break;
                default: Serial.println(F("Unknown status")); break;
            }
        } else {
            Serial.println(F("Failed to parse registration status"));
        }
    } else {
        Serial.println(F("Failed to get registration status"));
    }
}

void getOperatorName() {
    Serial.println(F("\n--- Operator ---"));

    sendATCommand("AT+COPS?", "+COPS:", 5000);

    // Parse operator name
    char* opsPtr = strstr(responseBuffer, "\"");
    if (opsPtr) {
        char* endPtr = strchr(opsPtr + 1, '"');
        if (endPtr) {
            *endPtr = '\0';
            Serial.print(F("Operator: "));
            Serial.println(opsPtr + 1);
            *endPtr = '"';  // Restore
        } else {
            Serial.println(F("Failed to parse operator name"));
        }
    } else {
        Serial.println(F("No operator (not registered)"));
    }
}

void manualATMode() {
    Serial.println(F("\n--- Manual AT Command Mode ---"));
    Serial.println(F("Type AT commands directly. Type 'exit' to quit."));
    Serial.println();

    String inputBuffer = "";

    while (true) {
        // Check for modem responses
        while (ModemSerial.available()) {
            char c = ModemSerial.read();
            Serial.write(c);
        }

        // Check for user input
        while (Serial.available()) {
            char c = Serial.read();

            if (c == '\r' || c == '\n') {
                if (inputBuffer.length() > 0) {
                    Serial.println();

                    if (inputBuffer.equalsIgnoreCase("exit")) {
                        Serial.println(F("Exiting manual mode"));
                        return;
                    }

                    ModemSerial.println(inputBuffer);
                    inputBuffer = "";
                }
            } else if (c == 127 || c == 8) {  // Backspace
                if (inputBuffer.length() > 0) {
                    inputBuffer.remove(inputBuffer.length() - 1);
                    Serial.write(8);
                    Serial.write(' ');
                    Serial.write(8);
                }
            } else {
                inputBuffer += c;
                Serial.write(c);
            }
        }

        delay(10);
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
    Serial.println(F("   COW-Bois Cellular Modem Test"));
    Serial.println(F("   SIM7600 4G LTE Module"));
    Serial.println(F("========================================="));
    Serial.println();

    // Initialize modem control pins
    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_RESET, OUTPUT);
    #if MODEM_POWER != 255
    pinMode(MODEM_POWER, OUTPUT);
    #endif

    digitalWrite(MODEM_PWRKEY, LOW);
    digitalWrite(MODEM_RESET, HIGH);
    #if MODEM_POWER != 255
    digitalWrite(MODEM_POWER, LOW);
    #endif

    // Initialize modem serial
    ModemSerial.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

    Serial.println(F("Modem pins initialized"));
    Serial.print(F("  TX: GPIO ")); Serial.println(MODEM_TX_PIN);
    Serial.print(F("  RX: GPIO ")); Serial.println(MODEM_RX_PIN);
    Serial.print(F("  PWRKEY: GPIO ")); Serial.println(MODEM_PWRKEY);
    Serial.print(F("  RESET: GPIO ")); Serial.println(MODEM_RESET);
    #if MODEM_POWER != 255
    Serial.print(F("  POWER: GPIO ")); Serial.println(MODEM_POWER);
    #else
    Serial.println(F("  POWER: Not configured"));
    #endif
    Serial.println();

    // Power on the modem
    powerOnModem();

    // Test communication
    testATCommunication();

    printHelp();
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();

        // Consume any extra characters (newline, etc)
        while (Serial.available()) {
            Serial.read();
        }

        switch (cmd) {
            case 'a':
            case 'A':
                testATCommunication();
                break;

            case 'i':
            case 'I':
                getModemInfo();
                break;

            case 'p':
            case 'P':
                checkSIMStatus();
                break;

            case 's':
            case 'S':
                checkSignalStrength();
                break;

            case 'n':
            case 'N':
                checkNetworkRegistration();
                break;

            case 'o':
            case 'O':
                getOperatorName();
                break;

            case 'r':
            case 'R':
                resetModem();
                testATCommunication();
                break;

            case 'w':
            case 'W':
                powerCycleModem();
                testATCommunication();
                break;

            case 'm':
            case 'M':
                manualATMode();
                printHelp();
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
