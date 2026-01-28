/**
 * COW-Bois Weather Station - HX711 Load Cell Test
 *
 * Tests the precipitation sensor (load cell for rain gauge).
 *
 * Upload: pio run -e test_hx711 -t upload
 * Monitor: pio device monitor
 *
 * Wiring:
 *   - HX711 DOUT: GPIO 16
 *   - HX711 SCK:  GPIO 17
 *
 * Tests:
 *   - Tare with empty container
 *   - Place known weight to calibrate
 *   - Convert weight to precipitation (mm)
 */

#include <Arduino.h>
#include <HX711.h>

// HX711 pins
#define HX711_DOUT 16
#define HX711_SCK 17

// Calibration defaults
#define DEFAULT_CALIBRATION_FACTOR 420.0
#define COLLECTOR_AREA_CM2 50.0  // Rain gauge collector area

HX711 scale;
bool sensorOK = false;

float calibrationFactor = DEFAULT_CALIBRATION_FACTOR;
float collectorArea = COLLECTOR_AREA_CM2;

float weightToPrecipitation(float weightGrams) {
    // Precipitation (mm) = Weight (g) / Area (cm²) * 10
    // Assumes water density ≈ 1 g/ml
    return (weightGrams / collectorArea) * 10.0;
}

void printReadings() {
    if (!sensorOK) {
        Serial.println("Sensor not initialized!");
        return;
    }

    if (!scale.is_ready()) {
        Serial.println("HX711 not ready! Check wiring.");
        return;
    }

    // Read raw value first, then calculate weight
    long rawValue = scale.read_average(5);
    float weight = (rawValue - scale.get_offset()) / calibrationFactor;
    if (weight < 0) weight = 0;  // Filter negative noise

    float precipitation = weightToPrecipitation(weight);

    Serial.println("========================================");
    Serial.printf("Raw Value:      %ld\n", rawValue);
    Serial.printf("Weight:         %.2f g\n", weight);
    Serial.printf("Precipitation:  %.2f mm\n", precipitation);
    Serial.printf("Calibration:    %.2f\n", calibrationFactor);
    Serial.printf("Collector Area: %.1f cm²\n", collectorArea);
    Serial.println("========================================");
}

void tare() {
    if (!sensorOK) return;

    Serial.println("\n*** TARING ***");
    Serial.println("Ensure collection vessel is EMPTY...");
    delay(1000);

    if (scale.is_ready()) {
        scale.tare(10);  // Average 10 readings
        Serial.println("Tare complete! Scale zeroed.");
        Serial.printf("New offset: %ld\n", scale.get_offset());
    } else {
        Serial.println("HX711 not ready! Tare failed.");
    }
    Serial.println();
}

void calibrate() {
    if (!sensorOK) return;

    Serial.println("\n*** CALIBRATION MODE ***");
    Serial.println("1. Ensure the scale is tared (empty vessel)");
    Serial.println("2. Place a KNOWN WEIGHT on the scale");
    Serial.println("3. Enter the weight in grams");
    Serial.println("\nPress any key when weight is placed...");

    while (!Serial.available()) delay(10);
    while (Serial.available()) Serial.read();  // Clear buffer

    // Get raw reading with weight
    Serial.println("Reading...");
    long rawWithWeight = scale.read_average(10);
    long offset = scale.get_offset();
    long rawDiff = rawWithWeight - offset;

    Serial.printf("Raw reading: %ld (diff from tare: %ld)\n", rawWithWeight, rawDiff);
    Serial.println("\nEnter the known weight in grams (e.g., 100):");

    while (!Serial.available()) delay(10);
    float knownWeight = Serial.parseFloat();

    if (knownWeight > 0 && rawDiff != 0) {
        calibrationFactor = (float)rawDiff / knownWeight;
        scale.set_scale(calibrationFactor);

        Serial.printf("\nCalibration complete!\n");
        Serial.printf("New calibration factor: %.2f\n", calibrationFactor);
        Serial.println("Save this value in config.h as PRECIP_CALIBRATION_FACTOR");
    } else {
        Serial.println("Invalid calibration values!");
    }
    Serial.println();
}

void testStability() {
    if (!sensorOK) return;

    Serial.println("\n*** STABILITY TEST ***");
    Serial.println("Taking 20 readings over 10 seconds...");
    Serial.println("Keep the load cell still.\n");

    float readings[20];
    float sum = 0;
    float minVal = 1e10;
    float maxVal = -1e10;

    for (int i = 0; i < 20; i++) {
        readings[i] = scale.get_units(3);
        sum += readings[i];
        if (readings[i] < minVal) minVal = readings[i];
        if (readings[i] > maxVal) maxVal = readings[i];
        Serial.printf("  Reading %2d: %.2f g\n", i + 1, readings[i]);
        delay(500);
    }

    float avg = sum / 20;
    float range = maxVal - minVal;

    Serial.println("\nResults:");
    Serial.printf("  Average: %.2f g\n", avg);
    Serial.printf("  Min:     %.2f g\n", minVal);
    Serial.printf("  Max:     %.2f g\n", maxVal);
    Serial.printf("  Range:   %.2f g\n", range);

    if (range < 1.0) {
        Serial.println("  Status:  EXCELLENT stability");
    } else if (range < 5.0) {
        Serial.println("  Status:  GOOD stability");
    } else {
        Serial.println("  Status:  POOR stability - check wiring/mounting");
    }
    Serial.println();
}

void printHelp() {
    Serial.println("\nCommands:");
    Serial.println("  'r' - Read sensor");
    Serial.println("  'c' - Continuous mode (1 sec interval)");
    Serial.println("  's' - Stop continuous mode");
    Serial.println("  't' - Tare (zero) scale");
    Serial.println("  'k' - Calibrate with known weight");
    Serial.println("  'x' - Test stability");
    Serial.println("  'p' - Power down HX711");
    Serial.println("  'u' - Power up HX711");
    Serial.println("  'a' - Set collector area");
    Serial.println("  'i' - Sensor info");
    Serial.println("  'h' - Help");
}

bool continuousMode = false;
unsigned long lastRead = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n========================================");
    Serial.println("COW-Bois HX711 Load Cell Test");
    Serial.println("========================================");
    Serial.printf("DOUT Pin: GPIO %d\n", HX711_DOUT);
    Serial.printf("SCK Pin:  GPIO %d\n", HX711_SCK);

    scale.begin(HX711_DOUT, HX711_SCK);

    delay(100);

    if (scale.is_ready()) {
        Serial.println("HX711 initialized successfully!");
        sensorOK = true;

        scale.set_scale(calibrationFactor);

        Serial.println("\nPerforming initial tare...");
        scale.tare(10);
        Serial.println("Tare complete.\n");

        printReadings();
    } else {
        Serial.println("HX711 initialization FAILED!");
        Serial.println("Check wiring:");
        Serial.printf("  DOUT -> GPIO %d\n", HX711_DOUT);
        Serial.printf("  SCK  -> GPIO %d\n", HX711_SCK);
        Serial.println("  VCC  -> 3.3V");
        Serial.println("  GND  -> GND");
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
            case 't':
            case 'T':
                tare();
                break;
            case 'k':
            case 'K':
                calibrate();
                break;
            case 'x':
            case 'X':
                testStability();
                break;
            case 'p':
            case 'P':
                scale.power_down();
                Serial.println("HX711 powered down");
                break;
            case 'u':
            case 'U':
                scale.power_up();
                delay(100);
                Serial.println("HX711 powered up");
                break;
            case 'a':
            case 'A':
                Serial.println("Enter collector area in cm² (e.g., 50):");
                while (!Serial.available()) delay(10);
                collectorArea = Serial.parseFloat();
                while (Serial.available()) Serial.read();  // Clear buffer
                Serial.printf("Collector area set to: %.1f cm²\n", collectorArea);
                break;
            case 'i':
            case 'I':
                Serial.println("\nSensor Info:");
                Serial.printf("  DOUT Pin: GPIO %d\n", HX711_DOUT);
                Serial.printf("  SCK Pin:  GPIO %d\n", HX711_SCK);
                Serial.printf("  Calibration Factor: %.2f\n", calibrationFactor);
                Serial.printf("  Collector Area: %.1f cm²\n", collectorArea);
                Serial.printf("  Offset: %ld\n", scale.get_offset());
                Serial.printf("  Ready: %s\n", scale.is_ready() ? "Yes" : "No");
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
