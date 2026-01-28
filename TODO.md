# COW-Bois Weather Station - Software TODO

**Firmware Version**: 1.0.1
**Last Updated**: 2026-01-28

## Build Status

- [x] All source files compile successfully
- [x] No compiler errors
- [x] RAM usage: ~14.5% (47KB / 320KB)
- [x] Flash usage: ~62.4% (817KB / 1.3MB)
- [x] All test sketches compile successfully

## Before First Upload

- [ ] Copy `secrets.h.template` to `secrets.h`
- [ ] Fill in MQTT broker credentials
- [ ] Fill in cellular APN settings
- [ ] Set main station MAC address (for microstations)
- [ ] Verify pin definitions match your hardware

## Test Sketches Available

Individual test sketches for hardware validation:

| Test | Command | Status |
|------|---------|--------|
| I2C Scanner | `pio run -e test_i2c_scan -t upload` | ✅ Ready |
| BME280 | `pio run -e test_bme280 -t upload` | ✅ Ready |
| TSL2591 | `pio run -e test_tsl2591 -t upload` | ✅ Ready |
| SGP30 | `pio run -e test_sgp30 -t upload` | ✅ Ready |
| Wind Sensors | `pio run -e test_wind -t upload` | ✅ Ready |
| HX711 Load Cell | `pio run -e test_hx711 -t upload` | ✅ Ready |

## Testing Checklist

### Sensor Testing (use test sketches)
- [ ] I2C scan finds devices at 0x76, 0x29, 0x58
- [ ] BME280 detected on I2C bus (`test_bme280`)
- [ ] BME280 temperature reading reasonable (15-30°C indoors)
- [ ] BME280 humidity reading reasonable (30-70% indoors)
- [ ] BME280 pressure reading reasonable (950-1050 hPa)
- [ ] TSL2591 detected on I2C bus (`test_tsl2591`)
- [ ] TSL2591 lux reading changes with light level
- [ ] SGP30 detected on I2C bus (`test_sgp30`)
- [ ] SGP30 CO2 baseline stabilizes after 15 seconds
- [ ] Wind speed ADC reading changes when flex sensor bent (`test_wind`)
- [ ] Wind direction ADC reading changes when vane rotated
- [ ] HX711 load cell tares successfully (`test_hx711`)
- [ ] HX711 weight reading changes with load

### Communication Testing
- [ ] Main station detects as MAIN_STATION mode
- [ ] Microstation detects as MICROSTATION mode
- [ ] ESP-NOW initializes successfully
- [ ] ESP-NOW packets received by main station
- [ ] Cellular modem responds to AT commands
- [ ] Cellular modem connects to network
- [ ] MQTT connection established
- [ ] MQTT data published successfully

### Power Testing
- [ ] Battery voltage ADC reading accurate
- [ ] Battery percentage calculation correct
- [ ] Low battery warning triggers
- [ ] Deep sleep enters/exits correctly

## Calibration Tasks

- [ ] Calibrate wind speed sensor with reference anemometer
- [ ] Calibrate wind direction sensor to true North
- [ ] Calibrate precipitation sensor with known weight
- [ ] Verify temperature accuracy against reference thermometer
- [ ] Store SGP30 baseline values after 12+ hour run

## Future Enhancements

### High Priority
- [ ] Add OTA (Over-The-Air) firmware updates
- [ ] Add SD card logging for data backup
- [ ] Implement watchdog timer for crash recovery
- [ ] Add SGP30 baseline persistence (SPIFFS/EEPROM)
- [ ] Add retry logic for failed cellular transmissions
- [ ] Buffer data locally when cellular unavailable

### Medium Priority
- [ ] Add web interface for configuration
- [ ] Add BLE for local configuration via phone
- [ ] Implement adaptive sampling rate based on conditions
- [ ] Add rainfall intensity calculation
- [ ] Add heat index calculation
- [ ] Add dew point calculation

### Low Priority
- [ ] Add sunrise/sunset detection for solar tracking
- [ ] Implement GPS for automatic location
- [ ] Add lightning detection sensor support
- [ ] Add soil moisture sensor support
- [ ] Add UV index sensor support

## Known Issues

1. **SGP30 warmup**: Requires 15 seconds after power-on for accurate readings
   - Current workaround: `isWarmedUp()` check before trusting readings

2. **Wind direction circular averaging**: Must handle 0°/360° boundary
   - Status: Implemented using sin/cos vector averaging

3. **ESP-NOW channel**: Must match between all devices
   - Status: Using channel 1 by default

4. **MQTT_MAX_PACKET_SIZE warning**: Compiler warning about redefinition
   - Status: Harmless - our config.h value (1024) overrides library default (256)
   - This is intentional to allow larger JSON payloads

5. **ESPNowHandler singleton pattern**: Only one instance should be created
   - Status: By design - ESP-NOW callbacks require static functions
   - Creating multiple instances will cause callback routing issues

## Code Quality Tasks

- [ ] Add unit tests for data aggregator
- [ ] Add unit tests for data formatter
- [x] Add hardware test sketches for sensors - COMPLETE (6 test sketches)
- [ ] Add static analysis (cppcheck)
- [x] Review and document all magic numbers (in config.h)
- [ ] Add error codes/enums for better debugging
- [x] Code review for header/implementation mismatches - COMPLETE
- [x] Null pointer safety checks in CellularModem - COMPLETE
- [x] Const correctness review - COMPLETE

## Documentation Tasks

- [x] Create README.md with project overview
- [x] Create SETUP.md with quick start guide
- [x] Create HARDWARE.md with wiring diagrams
- [x] Create TODO.md (this file)
- [ ] Add inline code documentation
- [ ] Create API reference for each module
- [x] Document MQTT payload schema (in README.md)
- [x] Create troubleshooting guide (in README.md and SETUP.md)

## Deployment Checklist

- [ ] All sensors calibrated
- [ ] Main station MAC address set in microstations
- [ ] MQTT broker accessible from cellular network
- [ ] Data visualization set up (Grafana/InfluxDB)
- [ ] Enclosure weatherproofed
- [ ] Solar panel positioned correctly
- [ ] Battery fully charged
- [ ] Mounting hardware secured
- [ ] Test transmission from deployment location
