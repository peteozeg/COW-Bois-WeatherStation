/**
 * COW-Bois Weather Station - Pin Definitions
 * Hardware pin mapping for ESP32-WROOM-32U
 */

#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

// ============================================
// I2C Bus (STEMMA QT Sensors)
// ============================================
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_SDA 21                    // Alias for main.cpp
#define I2C_SCL 22                    // Alias for main.cpp
#define I2C_FREQUENCY 100000          // 100kHz standard mode

// ============================================
// I2C Addresses
// ============================================
#define BME680_I2C_ADDR 0x76      // or 0x77 if SDO is HIGH
#define TSL2591_I2C_ADDR 0x29
#define SGP30_I2C_ADDR 0x58

// ============================================
// ADC Pins (Custom Sensors)
// ============================================
#define WIND_SPEED_ADC_PIN 34     // Flex sensor for wind speed
#define WIND_DIR_ADC_PIN 35       // Flex sensor for wind direction
#define PRECIPITATION_ADC_PIN 32  // HX711 data pin (if using ADC mode)
#define BATTERY_ADC_PIN 33        // Battery voltage monitor

// ============================================
// HX711 Load Cell (Precipitation)
// ============================================
#define HX711_DOUT_PIN 16
#define HX711_SCK_PIN 17

// ============================================
// Station Mode Selection
// ============================================
#define STATION_MODE_PIN 25       // LOW = Main Station, HIGH = Microstation

// ============================================
// Cellular Modem (LILYGO T-SIM7600)
// ============================================
#define MODEM_TX_PIN 27
#define MODEM_RX_PIN 26
#define MODEM_PWRKEY_PIN 4
#define MODEM_RESET_PIN 5
#define MODEM_POWER_PIN 23
#define MODEM_DTR_PIN 32
#define MODEM_RI_PIN 33

// ============================================
// Status LEDs (optional)
// ============================================
#define LED_STATUS_PIN 2          // Built-in LED on most ESP32 boards
#define LED_ERROR_PIN 13          // External error LED (optional)
#define LED_TRANSMIT_PIN 12       // Transmission indicator (optional)

// ============================================
// Power Control
// ============================================
#define SOLAR_ENABLE_PIN 14       // Enable/disable solar charging
#define SENSOR_POWER_PIN 15       // Power control for sensors
#define CHARGING_STATUS_PIN 36    // Charging status input (255 = not used)

// ============================================
// ADC Configuration
// ============================================
#define ADC_RESOLUTION 12         // 12-bit ADC (0-4095)
#define ADC_ATTEN ADC_11db        // Full 0-3.3V range
#define ADC_VREF 3.3f             // Reference voltage

// Voltage divider for battery monitoring
// If using a voltage divider (e.g., 100k/100k), set ratio here
#define BATTERY_DIVIDER_RATIO 2.0f

#endif // PIN_DEFINITIONS_H
