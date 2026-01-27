/**
 * COW-Bois Weather Station - Data Formatter
 * JSON and CSV formatting for weather data
 */

#ifndef DATA_FORMATTER_H
#define DATA_FORMATTER_H

#include <Arduino.h>
#include "data/weather_data.h"

class DataFormatter {
public:
    /**
     * Format single reading as JSON
     * @param reading Weather reading
     * @param buffer Output buffer
     * @param bufferSize Size of output buffer
     * @return Number of characters written
     */
    static size_t toJSON(const WeatherReading& reading, char* buffer, size_t bufferSize);

    /**
     * Format aggregated data as JSON
     * @param data Aggregated weather data
     * @param buffer Output buffer
     * @param bufferSize Size of output buffer
     * @return Number of characters written
     */
    static size_t toJSON(const AggregatedData& data, char* buffer, size_t bufferSize);

    /**
     * Format single reading as CSV line
     * @param reading Weather reading
     * @param buffer Output buffer
     * @param bufferSize Size of output buffer
     * @param includeHeader Include CSV header row
     * @return Number of characters written
     */
    static size_t toCSV(const WeatherReading& reading, char* buffer, size_t bufferSize,
                        bool includeHeader = false);

    /**
     * Format aggregated data as CSV line
     * @param data Aggregated weather data
     * @param buffer Output buffer
     * @param bufferSize Size of output buffer
     * @param includeHeader Include CSV header row
     * @return Number of characters written
     */
    static size_t toCSV(const AggregatedData& data, char* buffer, size_t bufferSize,
                        bool includeHeader = false);

    /**
     * Format aggregated data as MQTT payload
     * @param stationId Station identifier
     * @param data Aggregated weather data
     * @param buffer Output buffer
     * @param bufferSize Size of output buffer
     * @return Number of characters written
     */
    static size_t toMQTTPayload(const char* stationId, const AggregatedData& data,
                                char* buffer, size_t bufferSize);

    /**
     * Format data as InfluxDB line protocol
     * @param measurement Measurement name
     * @param stationId Station identifier (tag)
     * @param data Aggregated weather data
     * @param buffer Output buffer
     * @param bufferSize Size of output buffer
     * @return Number of characters written
     */
    static size_t toInfluxLineProtocol(const char* measurement, const char* stationId,
                                       const AggregatedData& data, char* buffer,
                                       size_t bufferSize);

    /**
     * Print weather reading to Serial (debug)
     * @param reading Weather reading
     */
    static void printReading(const WeatherReading& reading);

    /**
     * Print aggregated data to Serial (debug)
     * @param data Aggregated weather data
     */
    static void printAggregated(const AggregatedData& data);
};

#endif // DATA_FORMATTER_H
