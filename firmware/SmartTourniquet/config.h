#pragma once
#include <Arduino.h>

#define FW_VERSION_MAJOR  1
#define FW_VERSION_MINOR  0

// Set to 0 for field deployment to disable UART/USB power draw.
#define DEBUG_SERIAL 1

#if DEBUG_SERIAL
  #define DBG_BEGIN()  do { Serial.begin(115200); } while (0)
  #define DBG(...)     Serial.printf(__VA_ARGS__)
#else
  #define DBG_BEGIN()  do {} while (0)
  #define DBG(...)     do {} while (0)
#endif

// 1 = real time (1x); 60 = bench demo mode (1 min = 1 hour).
#define TIME_SCALE_FACTOR  1UL

#define NRF_PIN(port, pin)  ((port) * 32 + (pin))

#define PIN_REED_SWITCH  NRF_PIN(0, 22)
#define PIN_BUTTON       NRF_PIN(0, 24)
#define PIN_I2C_SDA      NRF_PIN(0, 17)
#define PIN_I2C_SCL      NRF_PIN(0, 20)
#define PIN_EXT_POWER    NRF_PIN(0, 13)  // Switches VCC rail to external peripherals
#define PIN_LED          NRF_PIN(0, 15)
// P0.09 / P0.10 are dedicated to NFCT coil and must not be assigned as GPIO.

// 0 = pulsed internal pull-up (~5 us per sample); 1 = external 1-2 MOhm resistor.
#define REED_EXTERNAL_PULLUP     0
#define REED_ARM_CONFIRM_MS      2000UL
#define REED_RELEASE_CONFIRM_MS  10000UL

#define BUTTON_DEBOUNCE_MS       30UL
#define BUTTON_LONG_PRESS_MS     5000UL
#define BUTTON_RESET_CONFIRM_MS  3000UL
#define DISPLAY_ON_MS            4000UL
#define DISPLAY_REFRESH_MS       1000UL
#define OLED_I2C_ADDR            0x3C
#define OLED_WIDTH               128
#define OLED_HEIGHT              64

// TCCC occlusion thresholds (seconds). Strict clinical limit is 120 minutes.
#define TRIAGE_ATTENTION_SEC     (90UL * 60UL)
#define TRIAGE_CRITICAL_SEC      (120UL * 60UL)

#define BLE_COMPANY_ID           0xFFFF
#define BLE_ADV_SAFE_MS          1500
#define BLE_ADV_ATTENTION_MS     1000
#define BLE_ADV_CRITICAL_MS      500
#define BLE_TX_POWER_DBM         0
#define BLE_PAYLOAD_UPDATE_MS    5000UL
#define BLE_DEVICE_NAME          "STQ"

#define BATTERY_LOW_PCT          20
#define BATTERY_MEASURE_MS       60000UL
#define BATTERY_EMA_ALPHA_NUM    1
#define BATTERY_EMA_ALPHA_DEN    4

#define PERSIST_INTERVAL_MS      60000UL
#define WDT_TIMEOUT_SEC          10UL
#define LOOP_IDLE_MS             100UL
