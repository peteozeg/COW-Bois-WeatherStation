/**
 * COW-Bois Weather Station - Wind Sensor Test
 *
 * Tests wind speed and direction flex sensors (ADC inputs).
 *
 * Upload: pio run -e test_wind -t upload
 * Monitor: pio device monitor
 *
 * Wiring:
 *   - Wind Speed: GPIO 34 (ADC input)
 *   - Wind Direction: GPIO 35 (ADC input)
 *   - Use voltage divider with flex sensors
 *
 * Tests:
 *   - Bend flex sensor: ADC value should change
 *   - Rotate direction vane: ADC should vary 0-4095
 */

#include <Arduino.h>

// ADC pins for wind sensors
#define WIND_SPEED_PIN 34
#define WIND_DIR_PIN 35

// ADC configuration
#define ADC_RESOLUTION 12      // 12-bit (0-4095)
#define ADC_VREF 3.3           // Reference voltage

// Calibration values (adjust based on your sensors)
float speedCalibrationFactor = 0.01;  // m/s per ADC unit
int directionOffset = 0;               // Raw ADC value for North

void printReadings() {
    int speedRaw = analogRead(WIND_SPEED_PIN);
    int dirRaw = analogRead(WIND_DIR_PIN);

    float speedVoltage = (speedRaw / 4095.0) * ADC_VREF;
    float dirVoltage = (dirRaw / 4095.0) * ADC_VREF;

    // Calculate wind speed (linear approximation)
    float windSpeed = speedRaw * speedCalibrationFactor;

    // Calculate wind direction (0-360 degrees)
    int correctedDir = dirRaw - directionOffset;
    if (correctedDir < 0) correctedDir += 4096;
    int windDir = map(correctedDir, 0, 4095, 0, 360);
    if (windDir >= 360) windDir -= 360;

    // Cardinal direction
    const char* cardinals[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
                               "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};
    int cardinalIndex = ((windDir + 11) % 360) / 22;
    if (cardinalIndex > 15) cardinalIndex = 15;

    Serial.println("========================================");
    Serial.println("Wind Speed Sensor:");
    Serial.printf("  Raw ADC:  %d (0-4095)\n", speedRaw);
    Serial.printf("  Voltage:  %.3f V\n", speedVoltage);
    Serial.printf("  Speed:    %.2f m/s (calibrated)\n", windSpeed);
    Serial.println();
    Serial.println("Wind Direction Sensor:");
    Serial.printf("  Raw ADC:  %d (0-4095)\n", dirRaw);
    Serial.printf("  Voltage:  %.3f V\n", dirVoltage);
    Serial.printf("  Direction: %d° %s\n", windDir, cardinals[cardinalIndex]);
    Serial.printf("  Offset:   %d (for North calibration)\n", directionOffset);
    Serial.println("========================================");
}

void calibrateNorth() {
    Serial.println("\n*** CALIBRATION MODE ***");
    Serial.println("Point the wind vane to TRUE NORTH");
    Serial.println("Press any key when ready...");

    while (!Serial.available()) delay(10);
    Serial.read();  // Clear the key

    // Take average of multiple readings
    long sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += analogRead(WIND_DIR_PIN);
        delay(50);
    }
    directionOffset = sum / 10;

    Serial.printf("North calibrated! Offset set to: %d\n", directionOffset);
    Serial.println("Direction readings are now relative to North.\n");
}

void testADCRange() {
    Serial.println("\n*** ADC RANGE TEST ***");
    Serial.println("Slowly rotate the direction vane through full range...");
    Serial.println("Press 's' to stop.\n");

    int minVal = 4095;
    int maxVal = 0;

    while (true) {
        if (Serial.available() && Serial.read() == 's') break;

        int val = analogRead(WIND_DIR_PIN);
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;

        Serial.printf("Current: %4d  Min: %4d  Max: %4d  Range: %4d\r",
                      val, minVal, maxVal, maxVal - minVal);
        delay(50);
    }

    Serial.printf("\n\nFinal Range: %d to %d (span: %d)\n", minVal, maxVal, maxVal - minVal);
    if (maxVal - minVal < 3000) {
        Serial.println("WARNING: Range seems limited. Check sensor connection.");
    }
    Serial.println();
}

void printHelp() {
    Serial.println("\nCommands:");
    Serial.println("  'r' - Read sensors");
    Serial.println("  'c' - Continuous mode (500ms interval)");
    Serial.println("  's' - Stop continuous mode");
    Serial.println("  'n' - Calibrate North direction");
    Serial.println("  't' - Test ADC range");
    Serial.println("  'f' - Set speed calibration factor");
    Serial.println("  'i' - Sensor info");
    Serial.println("  'h' - Help");
}

bool continuousMode = false;
unsigned long lastRead = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n========================================");
    Serial.println("COW-Bois Wind Sensor Test");
    Serial.println("========================================");
    Serial.printf("Speed Pin: GPIO %d (ADC)\n", WIND_SPEED_PIN);
    Serial.printf("Direction Pin: GPIO %d (ADC)\n", WIND_DIR_PIN);

    // Configure ADC
    analogReadResolution(ADC_RESOLUTION);
    analogSetAttenuation(ADC_11db);  // Full 0-3.3V range

    pinMode(WIND_SPEED_PIN, INPUT);
    pinMode(WIND_DIR_PIN, INPUT);

    Serial.println("\nSensor initialized.");
    Serial.println("Bend flex sensors or rotate vane to see changes.\n");

    printReadings();
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
                Serial.println("Continuous mode ON (500ms)");
                break;
            case 's':
            case 'S':
                continuousMode = false;
                Serial.println("Continuous mode OFF");
                break;
            case 'n':
            case 'N':
                calibrateNorth();
                break;
            case 't':
            case 'T':
                testADCRange();
                break;
            case 'f':
            case 'F':
                Serial.println("Enter speed calibration factor (e.g., 0.01):");
                while (!Serial.available()) delay(10);
                speedCalibrationFactor = Serial.parseFloat();
                while (Serial.available()) Serial.read();  // Clear buffer
                Serial.printf("Speed factor set to: %.4f\n", speedCalibrationFactor);
                break;
            case 'i':
            case 'I':
                Serial.println("\nSensor Info:");
                Serial.printf("  Speed Pin: GPIO %d\n", WIND_SPEED_PIN);
                Serial.printf("  Direction Pin: GPIO %d\n", WIND_DIR_PIN);
                Serial.printf("  ADC Resolution: %d-bit\n", ADC_RESOLUTION);
                Serial.printf("  Speed Factor: %.4f\n", speedCalibrationFactor);
                Serial.printf("  Direction Offset: %d\n", directionOffset);
                break;
            case 'h':
            case 'H':
            case '?':
                printHelp();
                break;
        }
    }

    if (continuousMode && (millis() - lastRead >= 500)) {
        lastRead = millis();
        printReadings();
    }

    delay(10);
}
