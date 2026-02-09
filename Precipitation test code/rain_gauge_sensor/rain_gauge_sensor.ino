#include <ESP32Servo.h>
#include <Arduino.h>

#define servoPin 2
#define adcPin_1 36

// Number of samples used for oversampling
#define OVERSAMPLES 150

Servo drainageServo;

// Servo position
int pos = 0;

// State flags
bool needToDrain = false;
bool draining = false;

// Averaged water sensor value
int waterValue = 0;

// Drain timing counter
int counter = 0;

// ADC threshold that triggers draining
int waterThreshold = 1725;

// Time between ADC samples (ms)
// This ALSO controls the oversampling rate
int checkDelay = 20;      // 20 ms × 150 samples ≈ 3.00 s per averaged reading

// Serial print rate
int printDelay = 3000;

// Timers
unsigned long timer = 0;
unsigned long printTimer = 0;
unsigned long drainTimer = 0;

// Oversampling accumulator variables
uint32_t adcSum = 0;      // Sum of ADC readings
uint16_t adcCount = 0;   // Number of samples collected

void setup() {
    Serial.begin(115200);
    Serial.println("Begin\n");

    delay(5000);  // startup delay only (safe)

    // Configure ESP32 ADC
    analogReadResolution(12);      // ADC range: 0–4095
    analogSetAttenuation(ADC_11db); // Full-scale ~3.3 V

    // Servo configuration
    drainageServo.setPeriodHertz(50);    // Standard servo PWM
    drainageServo.attach(servoPin, 1000, 2000);

    // Initialize timers
    timer = millis();
    printTimer = millis();
    drainTimer = millis();
}

void loop() {

    // -------- Non-blocking oversampled ADC read --------
    if (millis() - timer >= checkDelay) {

        // Take ONE ADC sample
        adcSum += analogRead(adcPin_1);
        adcCount++;

        // Once enough samples are collected
        if (adcCount >= OVERSAMPLES) {

            // Compute the averaged ADC value
            waterValue = adcSum / OVERSAMPLES;

            // Reset oversampling accumulators
            adcSum = 0;
            adcCount = 0;

            // Compare against threshold
            if (waterValue >= waterThreshold) {
                needToDrain = true;
            }
        }

        // Move timer forward without drift
        timer += checkDelay;
    }

    // -------- Serial output --------
    if (millis() - printTimer >= printDelay) {
        Serial.println(waterValue);
        printTimer += printDelay;
    }

    // -------- Drain state machine --------
    if (needToDrain && !draining) {
        drainTimer = millis();
        draining = true;
    }

    if (draining) {
        drainDevice();
    }

    // Continuously update servo position
    drainageServo.write(pos);
}

// -------- Drain control logic --------
void drainDevice() {

    // Clear drain request so it only triggers once
    needToDrain = false;

    // Move servo once at start of draining
    if (counter <= 0) {
        pos = 60;
    }

    // Increment drain timer every second
    if (millis() - drainTimer >= 1000) {
        counter++;
        Serial.println(counter);
        drainTimer += 1000;
    }

    // Stop draining after 20 seconds
    if (counter > 20) {
        counter = 0;
        pos = 0;
        draining = false;
    }
}
