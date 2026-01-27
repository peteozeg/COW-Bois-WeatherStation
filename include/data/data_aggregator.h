/**
 * COW-Bois Weather Station - Data Aggregator
 * Collects and averages sensor readings over transmission interval
 */

#ifndef DATA_AGGREGATOR_H
#define DATA_AGGREGATOR_H

#include <Arduino.h>
#include "config.h"
#include "data/weather_data.h"

// Data field enumeration for getCurrentAverage
enum class DataField {
    TEMPERATURE,
    HUMIDITY,
    PRESSURE,
    WIND_SPEED,
    WIND_DIRECTION,
    PRECIPITATION,
    LUX,
    SOLAR_IRRADIANCE,
    CO2,
    TVOC
};

class DataAggregator {
public:
    DataAggregator();

    /**
     * Add a sensor reading to the aggregation
     * @param reading Weather reading to add
     */
    void addSample(const WeatherReading& reading);

    /**
     * Check if aggregation window is complete
     * @return true if ready to transmit
     */
    bool isWindowComplete() const;

    /**
     * Get aggregated data (does not reset)
     * @return AggregatedData with calculated statistics
     */
    AggregatedData getAggregatedData();

    /**
     * Get aggregated data and reset for new window
     * @return AggregatedData with calculated statistics
     */
    AggregatedData getAndReset();

    /**
     * Reset aggregation (start new window)
     */
    void reset();

    /**
     * Get number of samples collected
     * @return Sample count
     */
    uint16_t getSampleCount() const;

    /**
     * Get elapsed time in current window
     * @return Elapsed time in milliseconds
     */
    uint32_t getWindowElapsedMs() const;

    /**
     * Get current average for a specific field
     * @param field Data field to query
     * @return Current average value
     */
    float getCurrentAverage(DataField field) const;

private:
    uint16_t _sampleCount;
    uint32_t _windowStartTime;

    // Running sums for averaging
    float _tempSum;
    float _tempMin;
    float _tempMax;

    float _humiditySum;
    float _humidityMin;
    float _humidityMax;

    float _pressureSum;
    float _pressureMin;
    float _pressureMax;

    float _windSpeedSum;
    float _windSpeedMax;

    // Wind direction circular averaging
    float _windDirSinSum;
    float _windDirCosSum;

    float _precipTotal;

    uint32_t _luxSum;
    uint32_t _luxMax;

    float _solarSum;

    uint32_t _co2Sum;
    uint16_t _co2Max;

    uint32_t _tvocSum;
    uint16_t _tvocMax;
};

#endif // DATA_AGGREGATOR_H
