/**
 * COW-Bois Weather Station - TSL2591 Simple Test
 *
 * Official Adafruit example code for TSL2591 light sensor.
 * Reads IR, full spectrum, visible light, and calculates lux.
 *
 * Upload: pio run -e test_tsl2591_simple -t upload
 * Monitor: pio device monitor
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

// I2C pins for ESP32
#define I2C_SDA 21
#define I2C_SCL 22

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

void displaySensorDetails(void)
{
    sensor_t sensor;
    tsl.getSensor(&sensor);
    Serial.println(F("------------------------------------"));
    Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
    Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
    Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
    Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println(F(" lux"));
    Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println(F(" lux"));
    Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution, 4); Serial.println(F(" lux"));
    Serial.println(F("------------------------------------"));
    Serial.println(F(""));
    delay(500);
}

void configureSensor(void)
{
    tsl.setGain(TSL2591_GAIN_MED);
    tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);

    Serial.println(F("------------------------------------"));
    Serial.print  (F("Gain:         "));
    tsl2591Gain_t gain = tsl.getGain();
    switch(gain)
    {
        case TSL2591_GAIN_LOW:
            Serial.println(F("1x (Low)"));
            break;
        case TSL2591_GAIN_MED:
            Serial.println(F("25x (Medium)"));
            break;
        case TSL2591_GAIN_HIGH:
            Serial.println(F("428x (High)"));
            break;
        case TSL2591_GAIN_MAX:
            Serial.println(F("9876x (Max)"));
            break;
    }
    Serial.print  (F("Timing:       "));
    Serial.print((tsl.getTiming() + 1) * 100, DEC);
    Serial.println(F(" ms"));
    Serial.println(F("------------------------------------"));
    Serial.println(F(""));
}

void advancedRead(void)
{
    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    Serial.print(F("[ ")); Serial.print(millis()); Serial.print(F(" ms ] "));
    Serial.print(F("IR: ")); Serial.print(ir);  Serial.print(F("  "));
    Serial.print(F("Full: ")); Serial.print(full); Serial.print(F("  "));
    Serial.print(F("Visible: ")); Serial.print(full - ir); Serial.print(F("  "));
    Serial.print(F("Lux: ")); Serial.println(tsl.calculateLux(full, ir), 6);
}

void setup(void)
{
    Serial.begin(115200);
    delay(2000);

    Serial.println(F("\n========================================"));
    Serial.println(F("COW-Bois TSL2591 Simple Test"));
    Serial.println(F("(Adafruit Example Code)"));
    Serial.println(F("========================================\n"));

    Wire.begin(I2C_SDA, I2C_SCL);

    if (tsl.begin())
    {
        Serial.println(F("Found a TSL2591 sensor"));
    }
    else
    {
        Serial.println(F("No sensor found ... check your wiring?"));
        while (1);
    }

    displaySensorDetails();
    configureSensor();
}

void loop(void)
{
    advancedRead();
    delay(500);
}
