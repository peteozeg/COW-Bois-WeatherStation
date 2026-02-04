/**
 * COW-Bois Weather Station - Weather Data Structures
 * Data types for sensor readings and aggregated data
 */

#ifndef WEATHER_DATA_H
#define WEATHER_DATA_H

#include <Arduino.h>

// ============================================
// Single Sensor Reading
// ============================================
struct WeatherReading {
    uint32_t timestamp;           // millis() timestamp

    // Temperature, Humidity, Pressure, Gas (BME680)
    float temperature;            // °C
    float humidity;               // % RH
    float pressure;               // hPa (mb)
    float gasResistance;          // KOhms (air quality indicator)

    // Solar Radiation (TSL2591)
    uint32_t lux;                 // Raw lux value
    float solarIrradiance;        // W/m²

    // Air Quality (SGP30)
    uint16_t co2;                 // ppm (equivalent CO2)
    uint16_t tvoc;                // ppb (Total VOC)

    // Wind (Flex sensors)
    float windSpeed;              // m/s
    uint16_t windDirection;       // degrees (0-359)

    // Precipitation (HX711 load cell)
    float precipitation;          // mm

    // Data validity flag
    bool isValid;

    // Default constructor
    WeatherReading() :
        timestamp(0),
        temperature(0), humidity(0), pressure(0), gasResistance(0),
        lux(0), solarIrradiance(0),
        co2(0), tvoc(0),
        windSpeed(0), windDirection(0),
        precipitation(0),
        isValid(false) {}
};

// ============================================
// Aggregated Data (5-minute interval)
// ============================================
struct AggregatedData {
    uint32_t timestamp;           // End timestamp
    uint32_t windowDurationMs;    // Duration of aggregation window
    uint16_t sampleCount;         // Number of samples averaged

    // Temperature
    float tempAvg;
    float tempMin;
    float tempMax;

    // Humidity
    float humidityAvg;
    float humidityMin;
    float humidityMax;

    // Pressure
    float pressureAvg;
    float pressureMin;
    float pressureMax;

    // Gas Resistance (BME680)
    float gasResistanceAvg;
    float gasResistanceMin;
    float gasResistanceMax;

    // Wind
    float windSpeedAvg;
    float windSpeedMax;           // Gust
    uint16_t windDirAvg;          // Circular average

    // Precipitation (cumulative)
    float precipitation;

    // Light
    uint32_t luxAvg;
    uint32_t luxMax;
    float solarAvg;

    // Air Quality
    uint16_t co2Avg;
    uint16_t co2Max;
    uint16_t tvocAvg;
    uint16_t tvocMax;

    // Default constructor
    AggregatedData() :
        timestamp(0), windowDurationMs(0), sampleCount(0),
        tempAvg(0), tempMin(999), tempMax(-999),
        humidityAvg(0), humidityMin(999), humidityMax(0),
        pressureAvg(0), pressureMin(9999), pressureMax(0),
        gasResistanceAvg(0), gasResistanceMin(9999), gasResistanceMax(0),
        windSpeedAvg(0), windSpeedMax(0), windDirAvg(0),
        precipitation(0),
        luxAvg(0), luxMax(0), solarAvg(0),
        co2Avg(0), co2Max(0), tvocAvg(0), tvocMax(0) {}
};

// ============================================
// ESP-NOW Packet Structure
// Must fit in 250 bytes (ESP-NOW limit)
// ============================================
struct __attribute__((packed)) ESPNowPacket {
    uint8_t packetType;           // 0x01 = weather data
    char stationId[9];            // Station identifier (null-terminated)
    uint32_t timestamp;

    // Compressed sensor data (scaled integers)
    int16_t temperature;          // °C * 100
    uint16_t humidity;            // % * 100
    uint16_t pressure;            // hPa * 10
    uint16_t gasResistance;       // KOhms * 10
    uint16_t windSpeed;           // m/s * 100
    uint16_t windDirection;       // degrees
    uint16_t precipitation;       // mm * 100
    uint32_t lux;                 // raw lux
    uint16_t co2;                 // ppm
    uint16_t tvoc;                // ppb
    uint16_t batteryVoltage;      // mV

    uint8_t flags;                // Bit flags (bit 0 = isValid)
    uint8_t checksum;             // Simple XOR checksum
};

// ============================================
// Sensor Status
// ============================================
struct SensorStatus {
    bool bme680_ok;
    bool tsl2591_ok;
    bool sgp30_ok;
    bool windSensor_ok;
    bool precipitation_ok;

    bool allOk() const {
        return bme680_ok && tsl2591_ok && sgp30_ok &&
               windSensor_ok && precipitation_ok;
    }
};

#endif // WEATHER_DATA_H
