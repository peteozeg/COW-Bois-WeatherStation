/**
 * COW-Bois Weather Station - BME680 Simple Test
 *
 * Official Adafruit example code for BME680 sensor testing.
 * Reads temperature, humidity, pressure, gas resistance, and altitude.
 *
 * Upload: pio run -e test_bme680_simple -t upload
 * Monitor: pio device monitor
 */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

// I2C pins for ESP32
#define I2C_SDA 21
#define I2C_SCL 22

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme;

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println(F("\n========================================"));
    Serial.println(F("COW-Bois BME680 Simple Test"));
    Serial.println(F("(Adafruit Example Code)"));
    Serial.println(F("========================================"));

    Wire.begin(I2C_SDA, I2C_SCL);

    if (!bme.begin()) {
        Serial.println("Could not find a valid BME680 sensor, check wiring!");
        while (1);
    }

    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320°C for 150 ms

    Serial.println("BME680 initialized successfully!");
    Serial.println("Reading every 2 seconds...\n");
}

void loop() {
    if (!bme.performReading()) {
        Serial.println("Failed to perform reading :(");
        return;
    }

    Serial.println("----------------------------------------");
    Serial.print("Temperature = ");
    Serial.print(bme.temperature);
    Serial.println(" °C");

    Serial.print("Pressure = ");
    Serial.print(bme.pressure / 100.0);
    Serial.println(" hPa");

    Serial.print("Humidity = ");
    Serial.print(bme.humidity);
    Serial.println(" %");

    Serial.print("Gas = ");
    Serial.print(bme.gas_resistance / 1000.0);
    Serial.println(" KOhms");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");
    Serial.println("----------------------------------------\n");

    delay(2000);
}
