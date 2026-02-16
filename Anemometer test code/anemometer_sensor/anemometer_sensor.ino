//#include <Arduino.h>

#define adcPin_NS 36
#define adcPin_EW 39

// Number of samples used for oversampling
#define oversamples 150

// Time between ADC samples (ms)
// This ALSO controls the oversampling rate
int checkDelay = 20;  // 20 ms × 150 samples ≈ 3.00 s per averaged reading

// Serial print rate
int printDelay = 3000;

// Readable variables
int magnitudeNS = 0;
int magnitudeEW = 0;

// Calibration Constants
int calibratedZeroNS = 0;
int calibratedZeroEW = 0;

// Magnitude used for calculations
signed int normalizedMagnitudeNS = 0;
signed int normalizedMagnitudeEW = 0;
double windMagnitude = 0;
double windDirection = 0;

// Timers
unsigned long timer = 0;
unsigned long printTimer = 0;

// Oversampling accumulator variables for ADC North and South
uint32_t adcSumNS = 0;  // Sum of ADC readings

// Oversampling accumulator variables for ADC East and West
uint32_t adcSumEW = 0;  // Sum of ADC readings

// Since the North and South and East and West directions are sampling at the same time
// We only need to count one counter
uint16_t adcCounter = 0;  // Number of samples collected

void setup() {
  Serial.begin(115200);
  Serial.println("Begin\n");

  delay(3000);  // startup delay only (safe)

  // Configure ESP32 ADC
  analogReadResolution(12);        // ADC range: 0–4095
  analogSetAttenuation(ADC_11db);  // Full-scale ~3.3 V

  // Initialize timers
  timer = millis();
  // Calibrate zero
  calibrateWind();
  printTimer = millis();
}

void loop() {

  // -------- Non-blocking oversampled ADC read --------
  if (millis() - timer >= checkDelay) {

    // Take ONE ADC sample
    adcSumNS += analogRead(adcPin_NS);
    adcSumEW += analogRead(adcPin_EW);
    adcCounter++;

    // Once enough samples are collected
    if (adcCounter >= oversamples) {

      // Compute the averaged ADC values
      magnitudeNS = adcSumNS / oversamples;
      magnitudeEW = adcSumEW / oversamples;

      // Get zero scaled values
      normalizedMagnitudeNS = magnitudeNS - calibratedZeroNS;
      normalizedMagnitudeEW = magnitudeEW - calibratedZeroEW;

      // Calculate Wind Speed and Direction
      windMagnitude = sqrt((normalizedMagnitudeNS*normalizedMagnitudeNS)+(normalizedMagnitudeEW*normalizedMagnitudeEW));
      windDirection = atan2(normalizedMagnitudeEW, normalizedMagnitudeNS);

      // Reset oversampling accumulators
      adcSumNS = 0;
      adcSumEW = 0;
      adcCounter = 0;
    }

    // Move timer forward without drift
    timer += checkDelay;
  }

  // -------- Serial output --------
  if (millis() - printTimer >= printDelay) {
    Serial.println("----- ADC Measurements -----");
    Serial.println();
    Serial.print("      NS = ");
    Serial.println(normalizedMagnitudeNS);
    Serial.println();
    Serial.print("      EW = ");
    Serial.println(normalizedMagnitudeEW);
    Serial.println();
    Serial.print("      Wind Speed = ");
    Serial.println(windMagnitude);
    Serial.println();
    Serial.print("      Direction = ");
    Serial.print(degrees(windDirection));
    Serial.println(" due North");
    Serial.println("--------------------------");
    Serial.println();

    // Should be close to the starting value (around 1800)
    Serial.println(calibratedZeroNS);
    Serial.println(calibratedZeroEW);
    Serial.println();

    printTimer += printDelay;
  }
}

void calibrateWind() {
  // Do Calibration when it is not windy (zero-ing out the values)
  int multiplyer = 3;
  bool calibrated = false;

  while (!calibrated) {
  if (millis() - timer >= checkDelay) {

    // Take ONE ADC sample
    adcSumNS += analogRead(adcPin_NS);
    adcSumEW += analogRead(adcPin_EW);
    adcCounter++;

    // Once enough samples are collected
    if (adcCounter >= (multiplyer * oversamples)) { // do twice so it has more settling time

      // Compute the averaged ADC values
      calibratedZeroNS = adcSumNS / (multiplyer * oversamples);
      calibratedZeroEW = adcSumEW / (multiplyer * oversamples);

      // Reset oversampling accumulators
      adcSumNS = 0;
      adcSumEW = 0;
      adcCounter = 0;
      calibrated = true;
    }

    // Move timer forward without drift
    timer += checkDelay;
  }
  }
  return;
}