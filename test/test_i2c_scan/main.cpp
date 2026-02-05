/**
 * COW-Bois Weather Station - I2C Bus Scanner
 *
 * Scans the I2C bus and reports all detected devices.
 * Use this first to verify all sensors are connected properly.
 *
 * Upload: pio run -e test_i2c_scan -t upload
 * Monitor: pio device monitor
 */

#include <Arduino.h>
#include <Wire.h>

// I2C pins (same as main firmware)
#define I2C_SDA 21
#define I2C_SCL 22

// Known device addresses
struct KnownDevice {
    uint8_t address;
    const char* name;
};

KnownDevice knownDevices[] = {
    {0x29, "TSL2591 (Light)"},
    {0x58, "SGP30 (Air Quality)"},
    {0x76, "BME680 (Temp/Humid/Press/Gas)"},
    {0x77, "BME680 (Alt Address)"},
    {0x48, "ADS1115 (ADC)"},
    {0x50, "EEPROM"},
};

const int numKnownDevices = sizeof(knownDevices) / sizeof(knownDevices[0]);

const char* getDeviceName(uint8_t address) {
    for (int i = 0; i < numKnownDevices; i++) {
        if (knownDevices[i].address == address) {
            return knownDevices[i].name;
        }
    }
    return "Unknown";
}

void scanI2C() {
    Serial.println("\n========================================");
    Serial.println("I2C Bus Scan");
    Serial.println("========================================");

    int devicesFound = 0;

    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            Serial.printf("  0x%02X - %s\n", address, getDeviceName(address));
            devicesFound++;
        } else if (error == 4) {
            Serial.printf("  0x%02X - ERROR (unknown)\n", address);
        }
    }

    Serial.println("----------------------------------------");
    Serial.printf("Total devices found: %d\n", devicesFound);
    Serial.println("========================================\n");

    // Check for expected devices
    Serial.println("Expected Devices Check:");

    // Check BME680
    Wire.beginTransmission(0x76);
    if (Wire.endTransmission() == 0) {
        Serial.println("  [OK] BME680 at 0x76");
    } else {
        Wire.beginTransmission(0x77);
        if (Wire.endTransmission() == 0) {
            Serial.println("  [OK] BME680 at 0x77 (alternate address)");
        } else {
            Serial.println("  [MISSING] BME680 - Check wiring!");
        }
    }

    // Check TSL2591
    Wire.beginTransmission(0x29);
    if (Wire.endTransmission() == 0) {
        Serial.println("  [OK] TSL2591 at 0x29");
    } else {
        Serial.println("  [MISSING] TSL2591 - Check wiring!");
    }

    // Check SGP30
    Wire.beginTransmission(0x58);
    if (Wire.endTransmission() == 0) {
        Serial.println("  [OK] SGP30 at 0x58");
    } else {
        Serial.println("  [MISSING] SGP30 - Check wiring!");
    }

    Serial.println();
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n========================================");
    Serial.println("COW-Bois I2C Bus Scanner");
    Serial.println("========================================");
    Serial.printf("SDA Pin: GPIO %d\n", I2C_SDA);
    Serial.printf("SCL Pin: GPIO %d\n", I2C_SCL);

    Wire.begin(I2C_SDA, I2C_SCL);

    scanI2C();

    Serial.println("Commands:");
    Serial.println("  's' - Scan again");
    Serial.println("  'r' - Reset I2C bus");
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();

        switch (cmd) {
            case 's':
            case 'S':
                scanI2C();
                break;
            case 'r':
            case 'R':
                Serial.println("Resetting I2C bus...");
                Wire.end();
                delay(100);
                Wire.begin(I2C_SDA, I2C_SCL);
                Serial.println("I2C bus reset complete");
                scanI2C();
                break;
        }
    }

    delay(100);
}
