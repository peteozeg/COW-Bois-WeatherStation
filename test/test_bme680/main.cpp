/**
 * COW-Bois Weather Station - BME680 Sensor Test
 *
 * Tests temperature, humidity, pressure, and gas resistance readings.
 *
 * Upload: pio run -e test_bme680 -t upload
 * Monitor: pio device monitor
 *
 * Expected indoor readings:
 *   Temperature: 15-30°C
 *   Humidity: 30-70%
 *   Pressure: 950-1050 hPa
 *   Gas Resistance: 10-300 KOhms (varies with air quality)
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME680.h>

// I2C pins
#define I2C_SDA 21
#define I2C_SCL 22
#define BME680_ADDR 0x77  // Default address (0x76 if SDO pin is grounded)

Adafruit_BME680 bme;
bool sensorOK = false;

// Calibration offsets
float tempOffset = 0.0;
float humidityOffset = 0.0;
float pressureOffset = 0.0;

void printReadings() {
    if (!sensorOK) {
        Serial.println("Sensor not initialized!");
        return;
    }

    if (!bme.performReading()) {
        Serial.println("Failed to perform reading!");
        return;
    }

    float temp = bme.temperature + tempOffset;
    float humidity = bme.humidity + humidityOffset;
    float pressure = bme.pressure / 100.0F + pressureOffset;
    float gasResistance = bme.gas_resistance / 1000.0F;  // Convert to KOhms

    Serial.println("----------------------------------------");
    Serial.printf("Temperature:    %.2f °C (%.2f °F)\n", temp, temp * 9/5 + 32);
    Serial.printf("Humidity:       %.2f %%\n", humidity);
    Serial.printf("Pressure:       %.2f hPa (%.2f inHg)\n", pressure, pressure * 0.02953);
    Serial.printf("Gas Resistance: %.2f KOhms\n", gasResistance);

    // Altitude estimate (assumes sea level pressure of 1013.25 hPa)
    float altitude = 44330.0 * (1.0 - pow(pressure / 1013.25, 0.1903));
    Serial.printf("Altitude:       %.1f m (estimated)\n", altitude);

    // Dew point calculation
    float dewPoint = temp - ((100 - humidity) / 5.0);
    Serial.printf("Dew Point:      %.2f °C\n", dewPoint);
    Serial.println("----------------------------------------");
}

void printHelp() {
    Serial.println("\nCommands:");
    Serial.println("  'r' - Read sensor");
    Serial.println("  'c' - Continuous mode (1 sec interval)");
    Serial.println("  's' - Stop continuous mode");
    Serial.println("  'i' - Sensor info");
    Serial.println("  't' - Set temperature offset");
    Serial.println("  'h' - Help");
}

bool continuousMode = false;
unsigned long lastRead = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n========================================");
    Serial.println("COW-Bois BME680 Sensor Test");
    Serial.println("========================================");

    Wire.begin(I2C_SDA, I2C_SCL);

    Serial.printf("Initializing BME680 at 0x%02X...\n", BME680_ADDR);

    if (bme.begin(BME680_ADDR, &Wire)) {
        Serial.println("BME680 initialized successfully!");
        sensorOK = true;

        // Configure oversampling and filter
        bme.setTemperatureOversampling(BME680_OS_16X);
        bme.setHumidityOversampling(BME680_OS_16X);
        bme.setPressureOversampling(BME680_OS_16X);
        bme.setIIRFilterSize(BME680_FILTER_SIZE_15);

        // Configure gas heater (320°C for 150ms)
        bme.setGasHeater(320, 150);

        // Print initial reading
        delay(500);
        printReadings();
    } else {
        Serial.println("BME680 initialization FAILED!");
        Serial.println("Check wiring and I2C address.");
        Serial.println("Try running I2C scanner first.");
    }

    printHelp();
}

void loop() {
    // Handle serial commands
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
                Serial.println("Continuous mode ON (1 sec interval)");
                break;
            case 's':
            case 'S':
                continuousMode = false;
                Serial.println("Continuous mode OFF");
                break;
            case 'i':
            case 'I':
                Serial.println("\nSensor Info:");
                Serial.printf("  BME680 at address 0x%02X\n", BME680_ADDR);
                Serial.printf("  Temp Offset: %.2f °C\n", tempOffset);
                Serial.printf("  Humidity Offset: %.2f %%\n", humidityOffset);
                Serial.printf("  Pressure Offset: %.2f hPa\n", pressureOffset);
                break;
            case 't':
            case 'T':
                Serial.println("Enter temperature offset (e.g., -1.5):");
                while (!Serial.available()) delay(10);
                tempOffset = Serial.parseFloat();
                while (Serial.available()) Serial.read();  // Clear buffer
                Serial.printf("Temperature offset set to: %.2f °C\n", tempOffset);
                break;
            case 'h':
            case 'H':
            case '?':
                printHelp();
                break;
        }
    }

    // Continuous mode readings
    if (continuousMode && sensorOK && (millis() - lastRead >= 1000)) {
        lastRead = millis();
        printReadings();
    }

    delay(10);
}
