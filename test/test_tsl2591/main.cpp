/**
 * COW-Bois Weather Station - TSL2591 Light Sensor Test
 *
 * Tests light/lux readings and solar irradiance estimation.
 *
 * Upload: pio run -e test_tsl2591 -t upload
 * Monitor: pio device monitor
 *
 * Tests:
 *   - Cover sensor: lux should drop to near 0
 *   - Shine light: lux should increase significantly
 *   - Outdoor sunlight: 10,000-100,000+ lux
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TSL2591.h>

// I2C pins
#define I2C_SDA 21
#define I2C_SCL 22

// Lux to W/m² conversion factor (approximate for sunlight)
#define LUX_TO_WM2 0.0079

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
bool sensorOK = false;

void printGainSetting() {
    tsl2591Gain_t gain = tsl.getGain();
    Serial.print("  Gain: ");
    switch (gain) {
        case TSL2591_GAIN_LOW:  Serial.println("1x (Low)"); break;
        case TSL2591_GAIN_MED:  Serial.println("25x (Medium)"); break;
        case TSL2591_GAIN_HIGH: Serial.println("428x (High)"); break;
        case TSL2591_GAIN_MAX:  Serial.println("9876x (Max)"); break;
    }
}

void printTimingSetting() {
    tsl2591IntegrationTime_t timing = tsl.getTiming();
    Serial.print("  Integration: ");
    switch (timing) {
        case TSL2591_INTEGRATIONTIME_100MS: Serial.println("100ms"); break;
        case TSL2591_INTEGRATIONTIME_200MS: Serial.println("200ms"); break;
        case TSL2591_INTEGRATIONTIME_300MS: Serial.println("300ms"); break;
        case TSL2591_INTEGRATIONTIME_400MS: Serial.println("400ms"); break;
        case TSL2591_INTEGRATIONTIME_500MS: Serial.println("500ms"); break;
        case TSL2591_INTEGRATIONTIME_600MS: Serial.println("600ms"); break;
    }
}

void printReadings() {
    if (!sensorOK) {
        Serial.println("Sensor not initialized!");
        return;
    }

    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir = lum >> 16;
    uint16_t full = lum & 0xFFFF;
    uint16_t visible = (full > ir) ? (full - ir) : 0;

    float lux = tsl.calculateLux(full, ir);
    float irradiance = lux * LUX_TO_WM2;

    Serial.println("----------------------------------------");
    Serial.printf("Full Spectrum: %u (raw)\n", full);
    Serial.printf("Infrared:      %u (raw)\n", ir);
    Serial.printf("Visible:       %u (raw)\n", visible);
    Serial.printf("Lux:           %.2f lx\n", lux);
    Serial.printf("Irradiance:    %.2f W/m² (estimated)\n", irradiance);

    // Light level description
    Serial.print("Light Level:   ");
    if (lux < 1) Serial.println("Dark");
    else if (lux < 50) Serial.println("Dim indoor");
    else if (lux < 500) Serial.println("Normal indoor");
    else if (lux < 1000) Serial.println("Bright indoor");
    else if (lux < 10000) Serial.println("Overcast outdoor");
    else if (lux < 50000) Serial.println("Cloudy outdoor");
    else Serial.println("Direct sunlight");

    Serial.println("----------------------------------------");
}

void autoGain() {
    uint32_t lum = tsl.getFullLuminosity();
    uint16_t full = lum & 0xFFFF;

    if (full < 100) {
        tsl.setGain(TSL2591_GAIN_MAX);
        Serial.println("Auto-gain set to MAX");
    } else if (full < 1000) {
        tsl.setGain(TSL2591_GAIN_HIGH);
        Serial.println("Auto-gain set to HIGH");
    } else if (full < 10000) {
        tsl.setGain(TSL2591_GAIN_MED);
        Serial.println("Auto-gain set to MEDIUM");
    } else {
        tsl.setGain(TSL2591_GAIN_LOW);
        Serial.println("Auto-gain set to LOW");
    }
}

void printHelp() {
    Serial.println("\nCommands:");
    Serial.println("  'r' - Read sensor");
    Serial.println("  'c' - Continuous mode (1 sec interval)");
    Serial.println("  's' - Stop continuous mode");
    Serial.println("  'a' - Auto-adjust gain");
    Serial.println("  '1' - Set gain LOW (1x)");
    Serial.println("  '2' - Set gain MEDIUM (25x)");
    Serial.println("  '3' - Set gain HIGH (428x)");
    Serial.println("  '4' - Set gain MAX (9876x)");
    Serial.println("  'i' - Sensor info");
    Serial.println("  'h' - Help");
}

bool continuousMode = false;
unsigned long lastRead = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n========================================");
    Serial.println("COW-Bois TSL2591 Light Sensor Test");
    Serial.println("========================================");

    Wire.begin(I2C_SDA, I2C_SCL);

    Serial.println("Initializing TSL2591...");

    if (tsl.begin()) {
        Serial.println("TSL2591 initialized successfully!");
        sensorOK = true;

        // Configure sensor
        tsl.setGain(TSL2591_GAIN_MED);
        tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);

        Serial.println("\nSensor Settings:");
        printGainSetting();
        printTimingSetting();

        delay(500);
        printReadings();
    } else {
        Serial.println("TSL2591 initialization FAILED!");
        Serial.println("Check wiring (should be at 0x29)");
    }

    printHelp();
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();

        switch (cmd) {
            case 'r':
            case 'R':
                printReadings();
                break;
            case 'c':
            case 'C':
                continuousMode = true;
                Serial.println("Continuous mode ON");
                break;
            case 's':
            case 'S':
                continuousMode = false;
                Serial.println("Continuous mode OFF");
                break;
            case 'a':
            case 'A':
                autoGain();
                break;
            case '1':
                tsl.setGain(TSL2591_GAIN_LOW);
                Serial.println("Gain set to LOW (1x)");
                break;
            case '2':
                tsl.setGain(TSL2591_GAIN_MED);
                Serial.println("Gain set to MEDIUM (25x)");
                break;
            case '3':
                tsl.setGain(TSL2591_GAIN_HIGH);
                Serial.println("Gain set to HIGH (428x)");
                break;
            case '4':
                tsl.setGain(TSL2591_GAIN_MAX);
                Serial.println("Gain set to MAX (9876x)");
                break;
            case 'i':
            case 'I':
                Serial.println("\nSensor Info:");
                Serial.println("  Sensor: TSL2591");
                Serial.println("  I2C Address: 0x29");
                printGainSetting();
                printTimingSetting();
                break;
            case 'h':
            case 'H':
            case '?':
                printHelp();
                break;
        }
    }

    if (continuousMode && sensorOK && (millis() - lastRead >= 1000)) {
        lastRead = millis();
        printReadings();
    }

    delay(10);
}
