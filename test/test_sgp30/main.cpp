/**
 * COW-Bois Weather Station - SGP30 Air Quality Sensor Test
 *
 * Tests CO2 and TVOC readings with warmup monitoring.
 *
 * Upload: pio run -e test_sgp30 -t upload
 * Monitor: pio device monitor
 *
 * Notes:
 *   - Requires 15 seconds warmup after power-on
 *   - Initial CO2 reading is 400 ppm (baseline)
 *   - Breathe near sensor to see CO2 spike
 *   - For accurate readings, run 12+ hours to establish baseline
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SGP30.h>

// I2C pins
#define I2C_SDA 21
#define I2C_SCL 22
#define WARMUP_TIME_MS 15000

Adafruit_SGP30 sgp;
bool sensorOK = false;
unsigned long startTime = 0;

// Baseline values (can be saved/restored for accuracy)
uint16_t baselineCO2 = 0;
uint16_t baselineTVOC = 0;

bool isWarmedUp() {
    return (millis() - startTime) >= WARMUP_TIME_MS;
}

// Calculate absolute humidity for compensation
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // Approximation formula
    const float absoluteHumidity = 216.7f *
        ((humidity / 100.0f) * 6.112f *
        exp((17.62f * temperature) / (243.12f + temperature)) /
        (273.15f + temperature));
    return static_cast<uint32_t>(1000.0f * absoluteHumidity);
}

void printReadings() {
    if (!sensorOK) {
        Serial.println("Sensor not initialized!");
        return;
    }

    if (!sgp.IAQmeasure()) {
        Serial.println("Measurement failed!");
        return;
    }

    Serial.println("----------------------------------------");

    if (!isWarmedUp()) {
        unsigned long remaining = (WARMUP_TIME_MS - (millis() - startTime)) / 1000;
        Serial.printf("WARMING UP... %lu seconds remaining\n", remaining);
        Serial.println("(Readings may not be accurate yet)");
    }

    Serial.printf("CO2:  %u ppm", sgp.eCO2);
    if (sgp.eCO2 == 400) Serial.print(" (baseline)");
    Serial.println();

    Serial.printf("TVOC: %u ppb\n", sgp.TVOC);

    // CO2 level description
    Serial.print("Air Quality: ");
    if (sgp.eCO2 < 600) Serial.println("Excellent");
    else if (sgp.eCO2 < 800) Serial.println("Good");
    else if (sgp.eCO2 < 1000) Serial.println("Fair");
    else if (sgp.eCO2 < 1500) Serial.println("Poor");
    else Serial.println("Very Poor - Ventilate!");

    Serial.println("----------------------------------------");
}

void printRawSignals() {
    if (!sensorOK) return;

    if (!sgp.IAQmeasureRaw()) {
        Serial.println("Raw measurement failed!");
        return;
    }

    Serial.println("Raw Signals:");
    Serial.printf("  H2:      %u\n", sgp.rawH2);
    Serial.printf("  Ethanol: %u\n", sgp.rawEthanol);
}

void getBaseline() {
    if (!sensorOK) return;

    if (sgp.getIAQBaseline(&baselineCO2, &baselineTVOC)) {
        Serial.println("Baseline values retrieved:");
        Serial.printf("  CO2 baseline:  0x%04X\n", baselineCO2);
        Serial.printf("  TVOC baseline: 0x%04X\n", baselineTVOC);
        Serial.println("Save these values to restore accuracy after power cycle.");
    } else {
        Serial.println("Failed to get baseline!");
    }
}

void setBaseline() {
    if (!sensorOK) return;

    if (baselineCO2 != 0 && baselineTVOC != 0) {
        if (sgp.setIAQBaseline(baselineCO2, baselineTVOC)) {
            Serial.println("Baseline restored successfully!");
        } else {
            Serial.println("Failed to set baseline!");
        }
    } else {
        Serial.println("No baseline values stored. Run 'g' first after 12+ hours.");
    }
}

void setHumidityCompensation() {
    Serial.println("\nEnter temperature (°C):");
    while (!Serial.available()) delay(10);
    float temp = Serial.parseFloat();
    while (Serial.available()) Serial.read();

    Serial.println("Enter humidity (%):");
    while (!Serial.available()) delay(10);
    float humid = Serial.parseFloat();
    while (Serial.available()) Serial.read();

    uint32_t absHumid = getAbsoluteHumidity(temp, humid);
    sgp.setHumidity(absHumid);

    Serial.printf("Humidity compensation set: %.1f°C, %.1f%% RH\n", temp, humid);
    Serial.printf("Absolute humidity: %lu mg/m³\n", absHumid);
    Serial.println("For best accuracy, update this periodically from BME680 readings.\n");
}

void printHelp() {
    Serial.println("\nCommands:");
    Serial.println("  'r' - Read sensor");
    Serial.println("  'c' - Continuous mode (1 sec interval)");
    Serial.println("  's' - Stop continuous mode");
    Serial.println("  'w' - Print raw H2/Ethanol signals");
    Serial.println("  'g' - Get baseline values (after 12+ hours)");
    Serial.println("  'b' - Restore baseline values");
    Serial.println("  'm' - Set humidity compensation (improves accuracy)");
    Serial.println("  'i' - Sensor info");
    Serial.println("  'h' - Help");
}

bool continuousMode = false;
unsigned long lastRead = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n========================================");
    Serial.println("COW-Bois SGP30 Air Quality Sensor Test");
    Serial.println("========================================");

    Wire.begin(I2C_SDA, I2C_SCL);

    Serial.println("Initializing SGP30...");

    if (sgp.begin()) {
        Serial.println("SGP30 initialized successfully!");
        sensorOK = true;
        startTime = millis();

        Serial.print("Serial #: ");
        Serial.print(sgp.serialnumber[0], HEX);
        Serial.print(sgp.serialnumber[1], HEX);
        Serial.println(sgp.serialnumber[2], HEX);

        Serial.println("\n*** WARMUP REQUIRED ***");
        Serial.printf("Please wait %d seconds for accurate readings.\n", WARMUP_TIME_MS / 1000);
        Serial.println("Initial readings will show baseline values.\n");

        delay(1000);
        printReadings();
    } else {
        Serial.println("SGP30 initialization FAILED!");
        Serial.println("Check wiring (should be at 0x58)");
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
            case 'w':
            case 'W':
                printRawSignals();
                break;
            case 'g':
            case 'G':
                getBaseline();
                break;
            case 'b':
            case 'B':
                setBaseline();
                break;
            case 'm':
            case 'M':
                setHumidityCompensation();
                break;
            case 'i':
            case 'I':
                Serial.println("\nSensor Info:");
                Serial.printf("  Warmed up: %s\n", isWarmedUp() ? "Yes" : "No");
                Serial.printf("  Uptime: %lu seconds\n", (millis() - startTime) / 1000);
                Serial.printf("  Baseline CO2: 0x%04X\n", baselineCO2);
                Serial.printf("  Baseline TVOC: 0x%04X\n", baselineTVOC);
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
