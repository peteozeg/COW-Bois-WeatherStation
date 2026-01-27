# COW-Bois Weather Station - Hardware Reference

## System Block Diagram

```
                                    ┌─────────────────┐
                                    │   Solar Panel   │
                                    └────────┬────────┘
                                             │
                                    ┌────────▼────────┐
                                    │ Charge Controller│
                                    └────────┬────────┘
                                             │
┌──────────────────────────────────┬─────────▼─────────┬──────────────────────────────────┐
│                                  │                   │                                  │
│  ┌─────────────┐                 │   ┌───────────┐   │                ┌─────────────┐   │
│  │   BME280    │◄────I2C────────►│   │           │   │◄───Serial────►│   SIM7600   │   │
│  │ Temp/Humid  │                 │   │   ESP32   │   │               │  4G Modem   │   │
│  └─────────────┘                 │   │  WROOM    │   │               └─────────────┘   │
│                                  │   │   32U     │   │                                  │
│  ┌─────────────┐                 │   │           │   │               ┌─────────────┐   │
│  │   TSL2591   │◄────I2C────────►│   │           │   │◄───ESP-NOW───►│ Microstation│   │
│  │   Light     │                 │   │           │   │               └─────────────┘   │
│  └─────────────┘                 │   └───────────┘   │                                  │
│                                  │         ▲         │                                  │
│  ┌─────────────┐                 │         │         │                                  │
│  │    SGP30    │◄────I2C────────►│         │         │                                  │
│  │ Air Quality │                 │         │         │                                  │
│  └─────────────┘                 │         │         │                                  │
│                                  │    ┌────┴────┐    │                                  │
│  ┌─────────────┐                 │    │   ADC   │    │                                  │
│  │ Flex Sensor │◄────ADC────────►│    │ Battery │    │                                  │
│  │ Wind Speed  │                 │    │ Monitor │    │                                  │
│  └─────────────┘                 │    └─────────┘    │                                  │
│                                  │                   │                                  │
│  ┌─────────────┐                 │                   │                                  │
│  │ Flex Sensor │◄────ADC────────►│                   │                                  │
│  │  Wind Dir   │                 │                   │                                  │
│  └─────────────┘                 │                   │                                  │
│                                  │                   │                                  │
│  ┌─────────────┐   ┌─────────┐   │                   │                                  │
│  │  Load Cell  │──►│  HX711  │──►│                   │                                  │
│  │ (Precip)    │   │   ADC   │   │                   │                                  │
│  └─────────────┘   └─────────┘   │                   │                                  │
│                                  │                   │                                  │
└──────────────────────────────────┴───────────────────┴──────────────────────────────────┘
```

## ESP32-WROOM-32U Pinout

```
                    ┌───────────────────┐
                    │    ESP32-WROOM    │
                    │       32U         │
                    │                   │
        3V3 ────────┤ 3V3          GND  ├──────── GND
         EN ────────┤ EN           IO23 ├──────── MODEM_POWER
  WIND_SPEED ───────┤ IO34 (ADC)   IO22 ├──────── I2C_SCL
   WIND_DIR ────────┤ IO35 (ADC)   IO1  ├──────── TX0
 BATTERY_ADC ───────┤ IO33 (ADC)   IO3  ├──────── RX0
 HX711_DOUT ────────┤ IO16         IO21 ├──────── I2C_SDA
  HX711_SCK ────────┤ IO17         GND  ├──────── GND
        GND ────────┤ GND          IO19 ├────────
   MODEM_RX ────────┤ IO26         IO18 ├────────
   MODEM_TX ────────┤ IO27         IO5  ├──────── MODEM_RESET
STATION_MODE ───────┤ IO25         IO17 ├──────── (HX711_SCK)
                    │              IO16 ├──────── (HX711_DOUT)
  MODEM_PWR ────────┤ IO4          IO4  ├──────── (MODEM_PWRKEY)
 STATUS_LED ────────┤ IO2          IO2  ├──────── (STATUS_LED)
                    │                   │
                    │     [USB-C]       │
                    └───────────────────┘
```

## I2C Bus Wiring (STEMMA QT)

All I2C sensors can be daisy-chained using STEMMA QT/Qwiic cables (4-pin JST).

```
ESP32                BME280              TSL2591             SGP30
┌─────┐             ┌─────┐             ┌─────┐             ┌─────┐
│ 3V3 ├─────────────┤ VIN ├─────────────┤ VIN ├─────────────┤ VIN │
│ GND ├─────────────┤ GND ├─────────────┤ GND ├─────────────┤ GND │
│ 21  ├─────────────┤ SDA ├─────────────┤ SDA ├─────────────┤ SDA │
│ 22  ├─────────────┤ SCL ├─────────────┤ SCL ├─────────────┤ SCL │
└─────┘             └─────┘             └─────┘             └─────┘
                    (0x76)              (0x29)              (0x58)
```

### Manual I2C Wiring
If not using STEMMA QT connectors:

| Wire Color (typical) | Signal | ESP32 GPIO |
|---------------------|--------|------------|
| Red | 3.3V | 3V3 |
| Black | GND | GND |
| Blue/Yellow | SDA | GPIO 21 |
| Yellow/White | SCL | GPIO 22 |

**Note:** Add 4.7kΩ pull-up resistors on SDA and SCL if not using breakout boards with built-in pull-ups.

## Wind Sensor Wiring

### Flex Sensor Voltage Divider
```
        3.3V
         │
         ┴
        ┌┴┐
        │ │ 10kΩ Fixed
        │ │ Resistor
        └┬┘
         ├──────────► To ESP32 ADC (GPIO 34 or 35)
        ┌┴┐
        │ │ Flex Sensor
        │ │ (variable)
        └┬┘
         │
        GND
```

### Connection Table
| Component | ESP32 Pin |
|-----------|-----------|
| Wind Speed Divider Output | GPIO 34 |
| Wind Direction Divider Output | GPIO 35 |

## HX711 Load Cell Wiring

```
Load Cell                    HX711                      ESP32
┌─────────┐                 ┌─────────┐                ┌─────────┐
│  RED    ├────── E+ ───────┤ E+      │                │         │
│  BLACK  ├────── E- ───────┤ E-      │                │         │
│  WHITE  ├────── A- ───────┤ A-      │                │         │
│  GREEN  ├────── A+ ───────┤ A+      │                │         │
└─────────┘                 │         │                │         │
                            │ VCC  ───┼────────────────┤ 3V3     │
                            │ GND  ───┼────────────────┤ GND     │
                            │ DOUT ───┼────────────────┤ GPIO 16 │
                            │ SCK  ───┼────────────────┤ GPIO 17 │
                            └─────────┘                └─────────┘
```

### Load Cell Wire Colors (typical)
| Color | Signal |
|-------|--------|
| Red | E+ (Excitation+) |
| Black | E- (Excitation-) |
| White | A- (Signal-) |
| Green | A+ (Signal+) |

## SIM7600 Cellular Modem

### LILYGO T-SIM7600G-H Connections
```
ESP32                           SIM7600 Module
┌─────────┐                     ┌─────────────┐
│ GPIO 27 ├─────── TX ──────────┤ RX          │
│ GPIO 26 ├─────── RX ──────────┤ TX          │
│ GPIO 23 ├─────── POWER ───────┤ POWER_ON    │
│ GPIO 5  ├─────── RESET ───────┤ RESET       │
│ GPIO 4  ├─────── PWRKEY ──────┤ PWRKEY      │
│ GND     ├─────── GND ─────────┤ GND         │
└─────────┘                     └─────────────┘
```

## Battery Monitoring

### Voltage Divider Circuit
```
Battery +  ──────┬──────
                 │
                ┌┴┐
                │ │ 100kΩ
                │ │
                └┬┘
                 ├──────────► To ESP32 ADC (GPIO 33)
                ┌┴┐
                │ │ 100kΩ
                │ │
                └┬┘
                 │
Battery -  ──────┴────── GND
```

**Calculation:**
- Divider ratio: 2:1
- Max measurable voltage: 3.3V × 2 = 6.6V
- For 4.2V LiPo: ADC reads ~2.1V

## Station Mode Selection

```
GPIO 25
   │
   ├───── NC ──────► HIGH = Microstation (pull-up)
   │
   └───── GND ─────► LOW = Main Station (jumper installed)
```

## Power System

### Solar Charging Circuit
```
Solar Panel ──────┐
(6V, 2W min)      │
                  ▼
           ┌──────────────┐
           │   TP4056     │
           │   Charge     │◄──── 3.7V LiPo Battery
           │  Controller  │      (2600mAh min)
           └──────┬───────┘
                  │
                  ▼
           ┌──────────────┐
           │   HT7333     │
           │   3.3V LDO   │──────► ESP32 3V3
           └──────────────┘
```

## LED Indicators (Optional)

| GPIO | LED | Function |
|------|-----|----------|
| GPIO 2 | Built-in | Status/heartbeat |
| GPIO 12 | External | Transmit indicator |
| GPIO 13 | External | Error indicator |

## Test Points

For debugging, consider adding test points:
- TP1: I2C SDA
- TP2: I2C SCL
- TP3: Battery voltage
- TP4: 3.3V rail
- TP5: GND

## PCB Design Considerations

1. **Keep analog traces short** - ADC inputs are noise-sensitive
2. **Separate analog and digital grounds** - Join at single point
3. **Add bypass capacitors** - 100nF near each IC
4. **Use ground plane** - Reduces EMI
5. **Consider conformal coating** - For outdoor deployment
