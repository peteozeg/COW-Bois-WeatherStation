# COW-Bois Weather Station - Quick Setup Guide

> **Build Status**: ✅ Firmware compiles successfully (v1.0.0)
>
> **Resource Usage**: RAM 14.5% | Flash 62.4%

## Prerequisites

- VS Code with PlatformIO IDE extension
- ESP32-WROOM-32U board
- USB cable for programming
- Sensors (BME280, TSL2591, SGP30, etc.)

## Step 1: Install Development Environment

1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Open VS Code Extensions (Ctrl+Shift+X)
3. Search for "PlatformIO IDE"
4. Click Install
5. Restart VS Code

## Step 2: Open Project

1. File → Open Folder
2. Navigate to `COW-Bois-WeatherStation`
3. Click "Select Folder"
4. Wait for PlatformIO to initialize (first time may take a few minutes)

## Step 3: Configure Credentials

```bash
# Copy the template
cp include/secrets.h.template include/secrets.h
```

Edit `include/secrets.h`:

```cpp
#ifndef SECRETS_H
#define SECRETS_H

// WiFi (for testing/development)
#define WIFI_SSID "YourWiFiName"
#define WIFI_PASSWORD "YourWiFiPassword"

// MQTT Broker
#define MQTT_BROKER "your-broker.com"
#define MQTT_PORT 1883
#define MQTT_USERNAME "your_username"
#define MQTT_PASSWORD "your_password"

// Cellular (Main Station)
#define CELLULAR_APN "your_carrier_apn"
#define CELLULAR_USER ""
#define CELLULAR_PASS ""

#endif
```

## Step 4: Configure Station Mode

### For Main Station:
- Connect GPIO 25 to GND (or leave jumper installed)
- The station will use cellular modem for data transmission

### For Microstation:
- Leave GPIO 25 floating (internal pull-up = HIGH)
- Edit `src/main.cpp` line 61 with main station's MAC address:
```cpp
uint8_t mainStationMAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
```

To find the main station's MAC address, upload firmware to main station first and check serial output:
```
ESP-NOW: MAC Address: AA:BB:CC:DD:EE:FF
```

## Step 5: Wire the Hardware

### I2C Sensors (STEMMA QT)
Simply daisy-chain using STEMMA QT cables:
```
ESP32 → BME280 → TSL2591 → SGP30
```

Or wire manually:
| ESP32 | Sensor |
|-------|--------|
| GPIO 21 | SDA |
| GPIO 22 | SCL |
| 3.3V | VIN |
| GND | GND |

### Wind Sensors (ADC)
| ESP32 | Sensor |
|-------|--------|
| GPIO 34 | Wind Speed Output |
| GPIO 35 | Wind Direction Output |
| 3.3V | VCC |
| GND | GND |

### Precipitation (HX711)
| ESP32 | HX711 |
|-------|-------|
| GPIO 16 | DOUT |
| GPIO 17 | SCK |
| 3.3V | VCC |
| GND | GND |

## Step 6: Build and Upload

### Using PlatformIO Toolbar (bottom of VS Code):
- ✓ (checkmark) = Build
- → (arrow) = Upload
- 🔌 (plug) = Serial Monitor

### Using Terminal:
```bash
# Build only
pio run

# Build and upload
pio run -t upload

# Open serial monitor
pio device monitor

# Build, upload, and monitor
pio run -t upload && pio device monitor
```

## Step 7: Verify Operation

Serial output should show:
```
========================================
COW-Bois Remote Weather Station
Kansas State University - ECE 591
========================================
Mode: MAIN_STATION (or MICROSTATION)
I2C initialized

Initializing sensors...
  BME280: OK
  TSL2591: OK
  SGP30: OK
  Wind Sensor: OK
  Precipitation: OK
Sensors initialized successfully

Setup complete. Starting measurements...
```

## Step 8: Calibrate Sensors

### Precipitation Sensor
1. Remove any water from collection vessel
2. Open serial monitor
3. Tare will happen automatically on startup
4. For manual calibration, modify main.cpp to call:
```cpp
sensors.getPrecipitation().calibrate(100.0);  // with 100g weight
```

### Wind Direction
1. Point vane to true North
2. Note the raw ADC value in serial output
3. Call calibration:
```cpp
sensors.getWindSensor().calibrateDirection(rawNorthValue);
```

## Troubleshooting

### "No such file: secrets.h"
You forgot to create secrets.h:
```bash
cp include/secrets.h.template include/secrets.h
```

### "BME280: FAILED"
- Check I2C wiring
- Try alternate address (0x77):
  Edit `include/pin_definitions.h`:
  ```cpp
  #define BME280_I2C_ADDR 0x77
  ```

### "Upload failed"
- Check USB connection
- Select correct port in PlatformIO
- Try pressing BOOT button on ESP32 during upload
- Install USB drivers if needed (CP2102 or CH340)

### "ESP-NOW send failed"
- Verify MAC addresses match
- Both devices must be powered on
- Check WiFi channel compatibility

### Garbage in Serial Monitor
- Set baud rate to 115200
- In PlatformIO: click monitor icon, then gear icon → set baud rate

## Next Steps

1. Set up MQTT broker (Mosquitto, HiveMQ Cloud, etc.)
2. Configure data visualization (Grafana, InfluxDB, etc.)
3. Deploy to field location
4. Set up solar power system
5. Weatherproof enclosure

## Debug Mode

To enable verbose debug output, ensure in `include/config.h`:
```cpp
#define DEBUG_ENABLED true
```

To disable (production), set to `false` to reduce power consumption.
