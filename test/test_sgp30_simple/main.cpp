/**
 * COW-Bois Weather Station - SGP30 Simple Test
 *
 * Official Adafruit example code for SGP30 air quality sensor.
 * Reads TVOC, eCO2, and raw H2/Ethanol values.
 *
 * Upload: pio run -e test_sgp30_simple -t upload
 * Monitor: pio device monitor
 */

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"

// I2C pins for ESP32
#define I2C_SDA 21
#define I2C_SCL 22

Adafruit_SGP30 sgp;

/* return absolute humidity [mg/m^3] with approximation formula
* @param temperature [°C]
* @param humidity [%RH]
*/
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature));
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);
    return absoluteHumidityScaled;
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println(F("\n========================================"));
    Serial.println(F("COW-Bois SGP30 Simple Test"));
    Serial.println(F("(Adafruit Example Code)"));
    Serial.println(F("========================================"));

    Wire.begin(I2C_SDA, I2C_SCL);

    if (!sgp.begin()) {
        Serial.println("Sensor not found :(");
        while (1);
    }
    Serial.print("Found SGP30 serial #");
    Serial.print(sgp.serialnumber[0], HEX);
    Serial.print(sgp.serialnumber[1], HEX);
    Serial.println(sgp.serialnumber[2], HEX);
    Serial.println();
}

int counter = 0;

void loop() {
    if (!sgp.IAQmeasure()) {
        Serial.println("Measurement failed");
        return;
    }
    Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
    Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");

    if (!sgp.IAQmeasureRaw()) {
        Serial.println("Raw Measurement failed");
        return;
    }
    Serial.print("Raw H2 "); Serial.print(sgp.rawH2); Serial.print(" \t");
    Serial.print("Raw Ethanol "); Serial.print(sgp.rawEthanol); Serial.println("");

    delay(1000);

    counter++;
    if (counter == 30) {
        counter = 0;

        uint16_t TVOC_base, eCO2_base;
        if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
            Serial.println("Failed to get baseline readings");
            return;
        }
        Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
        Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
    }
}
