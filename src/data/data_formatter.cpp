/**
 * COW-Bois Weather Station - Data Formatter Implementation
 */

#include "data/data_formatter.h"
#include "config.h"
#include <Arduino.h>

size_t DataFormatter::toJSON(const WeatherReading& reading, char* buffer, size_t bufferSize) {
    return snprintf(buffer, bufferSize,
        "{"
        "\"timestamp\":%lu,"
        "\"temperature\":%.2f,"
        "\"humidity\":%.2f,"
        "\"pressure\":%.2f,"
        "\"gas_resistance\":%.2f,"
        "\"wind_speed\":%.2f,"
        "\"wind_direction\":%u,"
        "\"precipitation\":%.2f,"
        "\"lux\":%lu,"
        "\"solar_irradiance\":%.2f,"
        "\"co2\":%u,"
        "\"tvoc\":%u,"
        "\"valid\":%s"
        "}",
        reading.timestamp,
        reading.temperature,
        reading.humidity,
        reading.pressure,
        reading.gasResistance,
        reading.windSpeed,
        reading.windDirection,
        reading.precipitation,
        reading.lux,
        reading.solarIrradiance,
        reading.co2,
        reading.tvoc,
        reading.isValid ? "true" : "false"
    );
}

size_t DataFormatter::toJSON(const AggregatedData& data, char* buffer, size_t bufferSize) {
    return snprintf(buffer, bufferSize,
        "{"
        "\"timestamp\":%lu,"
        "\"window_duration_ms\":%lu,"
        "\"sample_count\":%u,"
        "\"temperature\":{\"avg\":%.2f,\"min\":%.2f,\"max\":%.2f},"
        "\"humidity\":{\"avg\":%.2f,\"min\":%.2f,\"max\":%.2f},"
        "\"pressure\":{\"avg\":%.2f,\"min\":%.2f,\"max\":%.2f},"
        "\"gas_resistance\":{\"avg\":%.2f,\"min\":%.2f,\"max\":%.2f},"
        "\"wind\":{\"speed_avg\":%.2f,\"speed_max\":%.2f,\"direction_avg\":%u},"
        "\"precipitation\":%.2f,"
        "\"light\":{\"lux_avg\":%lu,\"lux_max\":%lu,\"solar_avg\":%.2f},"
        "\"air_quality\":{\"co2_avg\":%u,\"co2_max\":%u,\"tvoc_avg\":%u,\"tvoc_max\":%u}"
        "}",
        data.timestamp,
        data.windowDurationMs,
        data.sampleCount,
        data.tempAvg, data.tempMin, data.tempMax,
        data.humidityAvg, data.humidityMin, data.humidityMax,
        data.pressureAvg, data.pressureMin, data.pressureMax,
        data.gasResistanceAvg, data.gasResistanceMin, data.gasResistanceMax,
        data.windSpeedAvg, data.windSpeedMax, data.windDirAvg,
        data.precipitation,
        data.luxAvg, data.luxMax, data.solarAvg,
        data.co2Avg, data.co2Max, data.tvocAvg, data.tvocMax
    );
}

size_t DataFormatter::toCSV(const WeatherReading& reading, char* buffer, size_t bufferSize,
                            bool includeHeader) {
    size_t offset = 0;

    if (includeHeader) {
        offset += snprintf(buffer + offset, bufferSize - offset,
            "timestamp,temperature,humidity,pressure,gas_resistance,wind_speed,wind_direction,"
            "precipitation,lux,solar_irradiance,co2,tvoc,valid\n");
    }

    offset += snprintf(buffer + offset, bufferSize - offset,
        "%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%u,%.2f,%lu,%.2f,%u,%u,%d\n",
        reading.timestamp,
        reading.temperature,
        reading.humidity,
        reading.pressure,
        reading.gasResistance,
        reading.windSpeed,
        reading.windDirection,
        reading.precipitation,
        reading.lux,
        reading.solarIrradiance,
        reading.co2,
        reading.tvoc,
        reading.isValid ? 1 : 0
    );

    return offset;
}

size_t DataFormatter::toCSV(const AggregatedData& data, char* buffer, size_t bufferSize,
                            bool includeHeader) {
    size_t offset = 0;

    if (includeHeader) {
        offset += snprintf(buffer + offset, bufferSize - offset,
            "timestamp,window_ms,samples,"
            "temp_avg,temp_min,temp_max,"
            "humidity_avg,humidity_min,humidity_max,"
            "pressure_avg,pressure_min,pressure_max,"
            "gas_avg,gas_min,gas_max,"
            "wind_speed_avg,wind_speed_max,wind_dir_avg,"
            "precipitation,"
            "lux_avg,lux_max,solar_avg,"
            "co2_avg,co2_max,tvoc_avg,tvoc_max\n");
    }

    offset += snprintf(buffer + offset, bufferSize - offset,
        "%lu,%lu,%u,"
        "%.2f,%.2f,%.2f,"
        "%.2f,%.2f,%.2f,"
        "%.2f,%.2f,%.2f,"
        "%.2f,%.2f,%.2f,"
        "%.2f,%.2f,%u,"
        "%.2f,"
        "%lu,%lu,%.2f,"
        "%u,%u,%u,%u\n",
        data.timestamp,
        data.windowDurationMs,
        data.sampleCount,
        data.tempAvg, data.tempMin, data.tempMax,
        data.humidityAvg, data.humidityMin, data.humidityMax,
        data.pressureAvg, data.pressureMin, data.pressureMax,
        data.gasResistanceAvg, data.gasResistanceMin, data.gasResistanceMax,
        data.windSpeedAvg, data.windSpeedMax, data.windDirAvg,
        data.precipitation,
        data.luxAvg, data.luxMax, data.solarAvg,
        data.co2Avg, data.co2Max, data.tvocAvg, data.tvocMax
    );

    return offset;
}

size_t DataFormatter::toMQTTPayload(const char* stationId, const AggregatedData& data,
                                     char* buffer, size_t bufferSize) {
    return snprintf(buffer, bufferSize,
        "{"
        "\"station_id\":\"%s\","
        "\"timestamp\":%lu,"
        "\"data\":{"
        "\"temperature\":{\"value\":%.2f,\"min\":%.2f,\"max\":%.2f,\"unit\":\"C\"},"
        "\"humidity\":{\"value\":%.2f,\"min\":%.2f,\"max\":%.2f,\"unit\":\"%%\"},"
        "\"pressure\":{\"value\":%.2f,\"min\":%.2f,\"max\":%.2f,\"unit\":\"hPa\"},"
        "\"gas_resistance\":{\"value\":%.2f,\"min\":%.2f,\"max\":%.2f,\"unit\":\"KOhms\"},"
        "\"wind_speed\":{\"value\":%.2f,\"max\":%.2f,\"unit\":\"m/s\"},"
        "\"wind_direction\":{\"value\":%u,\"unit\":\"deg\"},"
        "\"precipitation\":{\"value\":%.2f,\"unit\":\"mm\"},"
        "\"solar_radiation\":{\"value\":%.2f,\"unit\":\"W/m2\"},"
        "\"co2\":{\"value\":%u,\"max\":%u,\"unit\":\"ppm\"},"
        "\"tvoc\":{\"value\":%u,\"max\":%u,\"unit\":\"ppb\"}"
        "},"
        "\"meta\":{\"samples\":%u,\"window_ms\":%lu}"
        "}",
        stationId,
        data.timestamp,
        data.tempAvg, data.tempMin, data.tempMax,
        data.humidityAvg, data.humidityMin, data.humidityMax,
        data.pressureAvg, data.pressureMin, data.pressureMax,
        data.gasResistanceAvg, data.gasResistanceMin, data.gasResistanceMax,
        data.windSpeedAvg, data.windSpeedMax,
        data.windDirAvg,
        data.precipitation,
        data.solarAvg,
        data.co2Avg, data.co2Max,
        data.tvocAvg, data.tvocMax,
        data.sampleCount,
        data.windowDurationMs
    );
}

size_t DataFormatter::toInfluxLineProtocol(const char* measurement, const char* stationId,
                                            const AggregatedData& data, char* buffer,
                                            size_t bufferSize) {
    // InfluxDB line protocol format:
    // measurement,tag1=value1,tag2=value2 field1=value1,field2=value2 timestamp

    return snprintf(buffer, bufferSize,
        "%s,station=%s "
        "temp_avg=%.2f,temp_min=%.2f,temp_max=%.2f,"
        "humidity_avg=%.2f,humidity_min=%.2f,humidity_max=%.2f,"
        "pressure_avg=%.2f,pressure_min=%.2f,pressure_max=%.2f,"
        "gas_avg=%.2f,gas_min=%.2f,gas_max=%.2f,"
        "wind_speed_avg=%.2f,wind_speed_max=%.2f,wind_dir=%u,"
        "precipitation=%.2f,"
        "lux_avg=%lu,lux_max=%lu,solar_avg=%.2f,"
        "co2_avg=%u,co2_max=%u,tvoc_avg=%u,tvoc_max=%u,"
        "samples=%u "
        "%lu000000",  // Convert ms to ns (InfluxDB default)
        measurement,
        stationId,
        data.tempAvg, data.tempMin, data.tempMax,
        data.humidityAvg, data.humidityMin, data.humidityMax,
        data.pressureAvg, data.pressureMin, data.pressureMax,
        data.gasResistanceAvg, data.gasResistanceMin, data.gasResistanceMax,
        data.windSpeedAvg, data.windSpeedMax, data.windDirAvg,
        data.precipitation,
        data.luxAvg, data.luxMax, data.solarAvg,
        data.co2Avg, data.co2Max, data.tvocAvg, data.tvocMax,
        data.sampleCount,
        data.timestamp
    );
}

void DataFormatter::printReading(const WeatherReading& reading) {
    Serial.println("--- Weather Reading ---");
    Serial.printf("Timestamp: %lu ms\n", reading.timestamp);
    Serial.printf("Temperature: %.2f °C\n", reading.temperature);
    Serial.printf("Humidity: %.2f %%\n", reading.humidity);
    Serial.printf("Pressure: %.2f hPa\n", reading.pressure);
    Serial.printf("Gas Resistance: %.2f KOhms\n", reading.gasResistance);
    Serial.printf("Wind: %.2f m/s @ %u°\n", reading.windSpeed, reading.windDirection);
    Serial.printf("Precipitation: %.2f mm\n", reading.precipitation);
    Serial.printf("Light: %lu lux (%.2f W/m²)\n", reading.lux, reading.solarIrradiance);
    Serial.printf("CO2: %u ppm, TVOC: %u ppb\n", reading.co2, reading.tvoc);
    Serial.printf("Valid: %s\n", reading.isValid ? "Yes" : "No");
    Serial.println("-----------------------");
}

void DataFormatter::printAggregated(const AggregatedData& data) {
    Serial.println("=== Aggregated Data ===");
    Serial.printf("Window: %lu ms, Samples: %u\n", data.windowDurationMs, data.sampleCount);
    Serial.printf("Temperature: %.2f °C (min: %.2f, max: %.2f)\n",
                  data.tempAvg, data.tempMin, data.tempMax);
    Serial.printf("Humidity: %.2f %% (min: %.2f, max: %.2f)\n",
                  data.humidityAvg, data.humidityMin, data.humidityMax);
    Serial.printf("Pressure: %.2f hPa (min: %.2f, max: %.2f)\n",
                  data.pressureAvg, data.pressureMin, data.pressureMax);
    Serial.printf("Gas Resistance: %.2f KOhms (min: %.2f, max: %.2f)\n",
                  data.gasResistanceAvg, data.gasResistanceMin, data.gasResistanceMax);
    Serial.printf("Wind: %.2f m/s avg (max: %.2f) @ %u°\n",
                  data.windSpeedAvg, data.windSpeedMax, data.windDirAvg);
    Serial.printf("Precipitation: %.2f mm\n", data.precipitation);
    Serial.printf("Solar: %.2f W/m² avg\n", data.solarAvg);
    Serial.printf("CO2: %u ppm avg (max: %u)\n", data.co2Avg, data.co2Max);
    Serial.printf("TVOC: %u ppb avg (max: %u)\n", data.tvocAvg, data.tvocMax);
    Serial.println("=======================");
}

