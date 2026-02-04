# COW-Bois Remote Weather Station

**Kansas State University - ECE 591 Senior Design Project**

| | |
|---|---|
| **Version** | 1.0.0 |
| **Platform** | ESP32-WROOM-32U |
| **Framework** | Arduino (PlatformIO) |
| **Build Status** | ✅ Passing |

A low-cost, solar-powered remote weather station designed to meet AASC Mesonet standards at a fraction of the cost of commercial solutions.

## Team Members
- Gantzen Miller (Hardware/Mechanical)
- Kennedy Jones (Hardware/Mechanical)
- Pete Ozegovic (Software)
- Ben Rogers (Software/Mechanical)
- Christian Evans (Power System/Hardware)
- Abdullah Ali (Power System)

## Project Overview

This weather station system consists of:
- **Main Station**: Full-featured station with cellular (4G LTE) connectivity via SIM7600 modem
- **Microstations**: Satellite stations that communicate with the main station via ESP-NOW

Both station types use the ESP32-WROOM-32U microcontroller and share the same sensor suite.

## Hardware Requirements

### Microcontroller
- ESP32-WROOM-32U (with external antenna connector)

### Sensors
| Sensor | Measurement | Interface | I2C Address |
|--------|-------------|-----------|-------------|
| BME680 | Temperature, Humidity, Pressure, Gas | I2C | 0x76 |
| TSL2591 | Light/Solar Radiation | I2C | 0x29 |
| SGP30 | CO2, TVOC (Air Quality) | I2C | 0x58 |
| Flex Sensors | Wind Speed & Direction | ADC | - |
| HX711 + Load Cell | Precipitation | GPIO | - |

### Communication
- **Main Station**: LILYGO T-SIM7600G-H (4G LTE modem)
- **Microstations**: ESP-NOW (built into ESP32)

### Power
- Solar panel + LiPo battery
- Battery monitoring via ADC

## Pin Configuration

### I2C Bus (STEMMA QT)
| Pin | GPIO |
|-----|------|
| SDA | 21 |
| SCL | 22 |

### ADC Sensors
| Sensor | GPIO |
|--------|------|
| Wind Speed | 34 |
| Wind Direction | 35 |
| Battery Voltage | 33 |

### HX711 Load Cell
| Pin | GPIO |
|-----|------|
| DOUT | 16 |
| SCK | 17 |

### Cellular Modem (SIM7600)
| Pin | GPIO |
|-----|------|
| TX | 27 |
| RX | 26 |
| Power | 23 |
| Reset | 5 |
| PWR Key | 4 |

### Station Mode
| Pin | GPIO | State |
|-----|------|-------|
| Mode Select | 25 | LOW = Main Station, HIGH = Microstation |

## Software Architecture

```
src/
├── main.cpp                 # Main firmware entry point
├── sensors/
│   ├── sensor_manager.cpp   # Unified sensor interface
│   ├── bme280_sensor.cpp    # Temperature/humidity/pressure
│   ├── tsl2591_sensor.cpp   # Light/solar radiation
│   ├── sgp30_sensor.cpp     # Air quality
│   ├── wind_sensor.cpp      # Flex sensor anemometer
│   └── precipitation.cpp    # HX711 load cell
├── communication/
│   ├── mqtt_handler.cpp     # MQTT client
│   ├── espnow_handler.cpp   # ESP-NOW protocol
│   └── cellular_modem.cpp   # SIM7600 interface
├── data/
│   ├── data_aggregator.cpp  # Sample averaging
│   └── data_formatter.cpp   # JSON/CSV formatting
└── system/
    ├── power_manager.cpp    # Battery monitoring
    └── station_mode.cpp     # Main/micro station config

include/
├── config.h                 # Central configuration
├── pin_definitions.h        # Hardware pin mapping
├── secrets.h.template       # Credentials template
├── sensors/                 # Sensor header files
├── communication/           # Communication headers
├── data/
│   ├── weather_data.h       # Data structures
│   ├── data_aggregator.h
│   └── data_formatter.h
└── system/                  # System headers

test/                        # Individual sensor test sketches
├── test_i2c_scan/           # I2C bus scanner
├── test_bme280/             # BME280 test (legacy)
├── test_bme680_simple/      # BME680 basic test (Adafruit example)
├── test_tsl2591/            # Light sensor test
├── test_tsl2591_simple/     # TSL2591 basic test (Adafruit example)
├── test_sgp30/              # Air quality sensor test
├── test_sgp30_simple/       # SGP30 basic test (Adafruit example)
├── test_wind/               # Wind sensor ADC test
└── test_hx711/              # Load cell test
```

## Test Sketches

Individual test sketches are provided to test each sensor in isolation before running the full firmware.

### Available Tests

| Test | Command | Purpose |
|------|---------|---------|
| I2C Scanner | `pio run -e test_i2c_scan -t upload` | Find all I2C devices on bus |
| BME280 | `pio run -e test_bme280 -t upload` | Test temp, humidity, pressure (legacy) |
| BME680 Simple | `pio run -e test_bme680_simple -t upload` | Basic BME680 test (Adafruit example) |
| TSL2591 | `pio run -e test_tsl2591 -t upload` | Test light sensor with gain control |
| TSL2591 Simple | `pio run -e test_tsl2591_simple -t upload` | Basic TSL2591 test (Adafruit example) |
| SGP30 | `pio run -e test_sgp30 -t upload` | Test CO2/TVOC with warmup monitor |
| SGP30 Simple | `pio run -e test_sgp30_simple -t upload` | Basic SGP30 test (Adafruit example) |
| Wind | `pio run -e test_wind -t upload` | Test wind ADC sensors |
| HX711 | `pio run -e test_hx711 -t upload` | Test load cell with calibration |

### Usage

```bash
# Step 1: Scan I2C bus to verify sensor connections
pio run -e test_i2c_scan -t upload && pio device monitor

# Step 2: Test individual sensors
pio run -e test_bme280 -t upload && pio device monitor
```

### Interactive Commands

Each test sketch supports serial commands:
- `r` - Read sensor once
- `c` - Continuous reading mode
- `s` - Stop continuous mode
- `h` - Show help/available commands
- Additional sensor-specific commands (calibration, tare, etc.)

## Setup Instructions

### 1. Install PlatformIO

Install VS Code and the PlatformIO IDE extension.

### 2. Clone/Open Project

Open the `COW-Bois-WeatherStation` folder in VS Code.

### 3. Configure Credentials

```bash
cp include/secrets.h.template include/secrets.h
```

Edit `include/secrets.h` with your credentials:
```cpp
#define WIFI_SSID "your_ssid"
#define WIFI_PASSWORD "your_password"
#define MQTT_BROKER "broker.example.com"
#define MQTT_USERNAME "your_username"
#define MQTT_PASSWORD "your_password"
#define CELLULAR_APN "your_apn"
```

### 4. Set Main Station MAC Address

For microstations, edit `src/main.cpp` line 61:
```cpp
uint8_t mainStationMAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
```

Replace with your main station's actual MAC address.

### 5. Build and Upload

```bash
# Build
pio run

# Upload to ESP32
pio run -t upload

# Monitor serial output
pio device monitor
```

## Configuration

Edit `include/config.h` to adjust:

| Setting | Default | Description |
|---------|---------|-------------|
| `SAMPLE_INTERVAL_MS` | 3000 | Sensor sampling interval (3 sec) |
| `TRANSMIT_INTERVAL_MS` | 300000 | Data transmission interval (5 min) |
| `ESPNOW_TRANSMIT_INTERVAL_MS` | 30000 | Microstation transmit interval (30 sec) |
| `DEBUG_ENABLED` | true | Enable serial debug output |

## Data Flow

### Main Station
1. Sample sensors every 3 seconds
2. Aggregate data over 5-minute window (min/max/avg)
3. Receive data from microstations via ESP-NOW
4. Transmit all data to MQTT broker via cellular

### Microstation
1. Sample sensors every 3 seconds
2. Aggregate data over 30-second window
3. Transmit to main station via ESP-NOW

## MQTT Topics

| Topic | Description |
|-------|-------------|
| `cowbois/weather/<station_id>/weather` | Weather data (JSON) |
| `cowbois/weather/<station_id>/status` | Station status |

### Sample JSON Payload
```json
{
  "station_id": "WX12345678",
  "timestamp": 1706300000,
  "data": {
    "temperature": {"value": 22.5, "min": 21.0, "max": 24.0, "unit": "C"},
    "humidity": {"value": 65.0, "min": 60.0, "max": 70.0, "unit": "%"},
    "pressure": {"value": 1013.25, "unit": "hPa"},
    "wind_speed": {"value": 3.5, "max": 8.2, "unit": "m/s"},
    "wind_direction": {"value": 225, "unit": "deg"},
    "precipitation": {"value": 0.0, "unit": "mm"},
    "solar_radiation": {"value": 450.0, "unit": "W/m2"},
    "co2": {"value": 420, "unit": "ppm"},
    "tvoc": {"value": 15, "unit": "ppb"}
  }
}
```

## Sensor Calibration

### Wind Speed Sensor
1. Set up reference anemometer
2. Call `windSensor.calibrateSpeed(referenceMps, adcReading)`
3. Update `_speedCalibrationFactor` if needed

### Wind Direction Sensor
1. Point vane to true North
2. Note ADC reading
3. Call `windSensor.calibrateDirection(trueNorthRawReading)`

### Precipitation Sensor (Load Cell)
1. Ensure collection vessel is empty
2. Call `precipSensor.tare()`
3. Place known weight (e.g., 100g)
4. Call `precipSensor.calibrate(100.0)`

### SGP30 Air Quality
The SGP30 requires 15 seconds warmup after power-on. For best accuracy:
1. Run continuously for 12+ hours to establish baseline
2. Save baseline values: `sgp30.getBaseline(co2Base, tvocBase)`
3. Restore on startup: `sgp30.setBaseline(co2Base, tvocBase)`

## Troubleshooting

### Sensor Not Detected
- Check I2C wiring (SDA/SCL)
- Verify I2C address with `Wire.beginTransmission()` scan
- Ensure STEMMA QT cables are fully seated

### No Cellular Connection
- Verify SIM card is inserted correctly
- Check antenna connection
- Confirm APN settings are correct
- Monitor AT command responses in debug output

### ESP-NOW Not Working
- Ensure both devices are on same WiFi channel
- Verify MAC addresses are correct
- Check that WiFi is in STA mode

### Erratic ADC Readings
- Add capacitor (100nF) near ADC input
- Use averaging in software
- Check for loose connections

## Sensor Specifications

| Measurement | Range | Accuracy | Resolution |
|-------------|-------|----------|------------|
| Temperature | -40 to +85°C | ±0.3°C | 0.01°C |
| Humidity | 0-100% RH | ±2% | 0.008% |
| Pressure | 300-1100 hPa | ±1 hPa | 0.18 Pa |
| Wind Speed | 0-50 m/s | ±0.3 m/s | 0.1 m/s |
| Wind Direction | 0-360° | ±3° | 1° |
| Precipitation | 0-500 mm | ±5% | 0.254 mm |
| Solar Radiation | 0-1500 W/m² | ±5% | 0.2 W/m² |
| CO2 | 400-60000 ppm | ±15% | 1 ppm |
| TVOC | 0-60000 ppb | ±15% | 1 ppb |

## Memory Usage

| Resource | Used | Available | Percentage |
|----------|------|-----------|------------|
| RAM | 47 KB | 320 KB | 14.5% |
| Flash | 817 KB | 1.3 MB | 62.4% |

## Dependencies

All dependencies are managed via PlatformIO and automatically installed:

| Library | Version | Purpose |
|---------|---------|---------|
| Adafruit BME280 | 2.2.4 | Temperature/humidity/pressure (legacy) |
| Adafruit BME680 | 2.1.4 | Temperature/humidity/pressure/gas sensor |
| Adafruit TSL2591 | 1.4.5 | Light sensor |
| Adafruit SGP30 | 2.0.3 | Air quality sensor |
| HX711 | 0.7.5 | Load cell ADC |
| PubSubClient | 2.8.0 | MQTT client |
| ArduinoJson | 7.0.0 | JSON serialization |

## License

This project was developed for Kansas State University ECE 591 Senior Design.

## References

- [AASC Mesonet Standards](business%20documentation/AASC%20Recommendations%20and%20Best%20Practices%20for%20Mesonets%20-%20Final,%20Ver%201.pdf)
- [ESP32 Datasheet](business%20documentation/esp32-wroom-32d_esp32-wroom-32u_datasheet_en.pdf)
- [KSU Mesonet Technical Overview](business%20documentation/KSU%20Mesonet%20Technical%20Overview%20(Standards).pdf)

## Changelog

### v1.0.2 (2026-02-03)
- Added simple sensor test sketches using official Adafruit example code
  - test_bme680_simple: Basic BME680 temperature/humidity/pressure/gas test
  - test_sgp30_simple: Basic SGP30 air quality test
  - test_tsl2591_simple: Basic TSL2591 light sensor test
- Switched from BME280 to BME680 sensor (adds gas/VOC sensing)
- Retained BME280 test for potential hardware fallback

### v1.0.1 (2026-01-28)
- Added individual sensor test sketches for hardware validation
- Added I2C bus scanner utility
- Fixed ESP-NOW callback signature for ESP32 Arduino framework
- Fixed TSL2591 isConnected() method
- Added null pointer safety checks in CellularModem
- Improved DataAggregator min/max initialization
- Updated documentation with test procedures

### v1.0.0 (2026-01-26)
- Initial release
- Complete sensor suite: BME280, TSL2591, SGP30, wind sensors, precipitation
- ESP-NOW communication for microstations
- Cellular modem support for main station
- MQTT data publishing
- Power management with battery monitoring
- Data aggregation with min/max/average calculations
