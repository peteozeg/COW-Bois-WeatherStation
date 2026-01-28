/**
 * COW-Bois Weather Station - BME280 Sensor Test
 *
 * Tests temperature, humidity, and pressure readings.
 *
 * Upload: pio run -e test_bme280 -t upload
 * Monitor: pio device monitor
 *
 * Expected indoor readings:
 *   Temperature: 15-30°C
 *   Humidity: 30-70%
 *   Pressure: 950-1050 hPa
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

// I2C pins
#define I2C_SDA 21
#define I2C_SCL 22
#define BME280_ADDR 0x76  // Try 0x77 if this doesn't work

Adafruit_BME280 bme;
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

    float temp = bme.readTemperature() + tempOffset;
    float humidity = bme.readHumidity() + humidityOffset;
    float pressure = bme.readPressure() / 100.0F + pressureOffset;

    Serial.println("----------------------------------------");
    Serial.printf("Temperature: %.2f °C (%.2f °F)\n", temp, temp * 9/5 + 32);
    Serial.printf("Humidity:    %.2f %%\n", humidity);
    Serial.printf("Pressure:    %.2f hPa (%.2f inHg)\n", pressure, pressure * 0.02953);

    // Altitude estimate (assumes sea level pressure of 1013.25 hPa)
    float altitude = 44330.0 * (1.0 - pow(pressure / 1013.25, 0.1903));
    Serial.printf("Altitude:    %.1f m (estimated)\n", altitude);

    // Dew point calculation
    float dewPoint = temp - ((100 - humidity) / 5.0);
    Serial.printf("Dew Point:   %.2f °C\n", dewPoint);
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
    Serial.println("COW-Bois BME280 Sensor Test");
    Serial.println("========================================");

    Wire.begin(I2C_SDA, I2C_SCL);

    Serial.printf("Initializing BME280 at 0x%02X...\n", BME280_ADDR);

    if (bme.begin(BME280_ADDR, &Wire)) {
        Serial.println("BME280 initialized successfully!");
        sensorOK = true;

        // Configure for weather monitoring
        bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                        Adafruit_BME280::SAMPLING_X16,  // temperature
                        Adafruit_BME280::SAMPLING_X16,  // pressure
                        Adafruit_BME280::SAMPLING_X16,  // humidity
                        Adafruit_BME280::FILTER_X16,
                        Adafruit_BME280::STANDBY_MS_500);

        // Print initial reading
        delay(500);
        printReadings();
    } else {
        Serial.println("BME280 initialization FAILED!");
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
                Serial.printf("  Sensor ID: 0x%02X\n", bme.sensorID());
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
