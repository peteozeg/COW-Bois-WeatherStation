/**
 * COW-Bois Weather Station - Data Aggregator Implementation
 */

#include "data/data_aggregator.h"
#include "config.h"
#include <math.h>
#include <float.h>

DataAggregator::DataAggregator()
    : _sampleCount(0)
    , _windowStartTime(0) {
    reset();
}

void DataAggregator::reset() {
    _sampleCount = 0;
    _windowStartTime = millis();

    // Reset accumulated values
    _tempSum = 0;
    _tempMin = FLT_MAX;
    _tempMax = -FLT_MAX;

    _humiditySum = 0;
    _humidityMin = FLT_MAX;
    _humidityMax = 0;

    _pressureSum = 0;
    _pressureMin = FLT_MAX;
    _pressureMax = 0;

    _gasResistanceSum = 0;
    _gasResistanceMin = FLT_MAX;
    _gasResistanceMax = 0;

    _windSpeedSum = 0;
    _windSpeedMax = 0;

    // Wind direction uses circular averaging
    _windDirSinSum = 0;
    _windDirCosSum = 0;

    _precipTotal = 0;

    _luxSum = 0;
    _luxMax = 0;

    _solarSum = 0;

    _co2Sum = 0;
    _co2Max = 0;

    _tvocSum = 0;
    _tvocMax = 0;
}

void DataAggregator::addSample(const WeatherReading& reading) {
    if (!reading.isValid) return;

    _sampleCount++;

    // Temperature
    _tempSum += reading.temperature;
    if (reading.temperature < _tempMin) _tempMin = reading.temperature;
    if (reading.temperature > _tempMax) _tempMax = reading.temperature;

    // Humidity
    _humiditySum += reading.humidity;
    if (reading.humidity < _humidityMin) _humidityMin = reading.humidity;
    if (reading.humidity > _humidityMax) _humidityMax = reading.humidity;

    // Pressure
    _pressureSum += reading.pressure;
    if (reading.pressure < _pressureMin) _pressureMin = reading.pressure;
    if (reading.pressure > _pressureMax) _pressureMax = reading.pressure;

    // Gas Resistance
    _gasResistanceSum += reading.gasResistance;
    if (reading.gasResistance < _gasResistanceMin) _gasResistanceMin = reading.gasResistance;
    if (reading.gasResistance > _gasResistanceMax) _gasResistanceMax = reading.gasResistance;

    // Wind speed
    _windSpeedSum += reading.windSpeed;
    if (reading.windSpeed > _windSpeedMax) _windSpeedMax = reading.windSpeed;

    // Wind direction (circular average using vector components)
    float dirRad = reading.windDirection * M_PI / 180.0f;
    _windDirSinSum += sin(dirRad);
    _windDirCosSum += cos(dirRad);

    // Precipitation (cumulative)
    _precipTotal = reading.precipitation;  // Use latest value (cumulative from sensor)

    // Light
    _luxSum += reading.lux;
    if (reading.lux > _luxMax) _luxMax = reading.lux;

    // Solar irradiance
    _solarSum += reading.solarIrradiance;

    // CO2
    _co2Sum += reading.co2;
    if (reading.co2 > _co2Max) _co2Max = reading.co2;

    // TVOC
    _tvocSum += reading.tvoc;
    if (reading.tvoc > _tvocMax) _tvocMax = reading.tvoc;
}

bool DataAggregator::isWindowComplete() const {
    return (millis() - _windowStartTime) >= AGGREGATION_WINDOW_MS;
}

AggregatedData DataAggregator::getAggregatedData() {
    AggregatedData data;

    data.timestamp = millis();
    data.sampleCount = _sampleCount;
    data.windowDurationMs = millis() - _windowStartTime;

    if (_sampleCount == 0) {
        // No data collected
        data.tempAvg = 0;
        data.tempMin = 0;
        data.tempMax = 0;
        data.humidityAvg = 0;
        data.humidityMin = 0;
        data.humidityMax = 0;
        data.pressureAvg = 0;
        data.pressureMin = 0;
        data.pressureMax = 0;
        data.gasResistanceAvg = 0;
        data.gasResistanceMin = 0;
        data.gasResistanceMax = 0;
        data.windSpeedAvg = 0;
        data.windSpeedMax = 0;
        data.windDirAvg = 0;
        data.precipitation = 0;
        data.luxAvg = 0;
        data.luxMax = 0;
        data.solarAvg = 0;
        data.co2Avg = 0;
        data.co2Max = 0;
        data.tvocAvg = 0;
        data.tvocMax = 0;
        return data;
    }

    // Calculate averages
    data.tempAvg = _tempSum / _sampleCount;
    data.tempMin = _tempMin;
    data.tempMax = _tempMax;

    data.humidityAvg = _humiditySum / _sampleCount;
    data.humidityMin = _humidityMin;
    data.humidityMax = _humidityMax;

    data.pressureAvg = _pressureSum / _sampleCount;
    data.pressureMin = _pressureMin;
    data.pressureMax = _pressureMax;

    data.gasResistanceAvg = _gasResistanceSum / _sampleCount;
    data.gasResistanceMin = _gasResistanceMin;
    data.gasResistanceMax = _gasResistanceMax;

    data.windSpeedAvg = _windSpeedSum / _sampleCount;
    data.windSpeedMax = _windSpeedMax;

    // Calculate average wind direction from vector components
    float avgSin = _windDirSinSum / _sampleCount;
    float avgCos = _windDirCosSum / _sampleCount;
    float avgDir = atan2(avgSin, avgCos) * 180.0f / M_PI;
    if (avgDir < 0) avgDir += 360.0f;
    data.windDirAvg = (uint16_t)avgDir;

    data.precipitation = _precipTotal;

    data.luxAvg = _luxSum / _sampleCount;
    data.luxMax = _luxMax;

    data.solarAvg = _solarSum / _sampleCount;

    data.co2Avg = _co2Sum / _sampleCount;
    data.co2Max = _co2Max;

    data.tvocAvg = _tvocSum / _sampleCount;
    data.tvocMax = _tvocMax;

    return data;
}

AggregatedData DataAggregator::getAndReset() {
    AggregatedData data = getAggregatedData();
    reset();
    return data;
}

uint16_t DataAggregator::getSampleCount() const {
    return _sampleCount;
}

uint32_t DataAggregator::getWindowElapsedMs() const {
    return millis() - _windowStartTime;
}

float DataAggregator::getCurrentAverage(DataField field) const {
    if (_sampleCount == 0) return 0;

    switch (field) {
        case DataField::TEMPERATURE:
            return _tempSum / _sampleCount;
        case DataField::HUMIDITY:
            return _humiditySum / _sampleCount;
        case DataField::PRESSURE:
            return _pressureSum / _sampleCount;
        case DataField::GAS_RESISTANCE:
            return _gasResistanceSum / _sampleCount;
        case DataField::WIND_SPEED:
            return _windSpeedSum / _sampleCount;
        case DataField::WIND_DIRECTION: {
            float avgSin = _windDirSinSum / _sampleCount;
            float avgCos = _windDirCosSum / _sampleCount;
            float avgDir = atan2(avgSin, avgCos) * 180.0f / M_PI;
            if (avgDir < 0) avgDir += 360.0f;
            return avgDir;
        }
        case DataField::PRECIPITATION:
            return _precipTotal;
        case DataField::LUX:
            return (float)(_luxSum / _sampleCount);
        case DataField::SOLAR_IRRADIANCE:
            return _solarSum / _sampleCount;
        case DataField::CO2:
            return (float)(_co2Sum / _sampleCount);
        case DataField::TVOC:
            return (float)(_tvocSum / _sampleCount);
        default:
            return 0;
    }
}

