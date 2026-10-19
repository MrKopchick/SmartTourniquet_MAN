/**
 * @file config.h
 * @brief Centralized firmware configuration for SmartTourniquet.
 * All modules must consume parameters defined here. No local hardcoded overrides.
 */
#pragma once
#include <Arduino.h>

// ============================== Version ==============================
#define FW_VERSION_MAJOR  0
#define FW_VERSION_MINOR  1

// ============================== Debug ================================
// 1 = UART serial enabled (bench testing only).
// 0 = Production/field build: Serial disabled to reduce power consumption.
#define DEBUG_SERIAL 1

#if DEBUG_SERIAL
  #define DBG_BEGIN()  do { Serial.begin(115200); } while (0)
  #define DBG(...)     Serial.printf(__VA_ARGS__)
#else
  #define DBG_BEGIN()  do {} while (0)
  #define DBG(...)     do {} while (0)
#endif

// =================== Pinout (Pro Micro / nice!nano nRF52840) =========
// GPIO index calculation: port * 32 + pin.
// TODO: Verify target board variant pin-mapping (g_ADigitalPinMap).
#define NRF_GPIO(port, pin)  ((port) * 32 + (pin))

#define PIN_REED_SWITCH  NRF_GPIO(0, 22)  // Reed switch NO -> GND, pull-up
#define PIN_BUTTON       NRF_GPIO(0, 24)  // Tactile push button -> GND, pull-up
#define PIN_I2C_SDA      NRF_GPIO(0, 17)
#define PIN_I2C_SCL      NRF_GPIO(0, 20)
#define PIN_EXT_POWER    NRF_GPIO(0, 13)  // nice!nano external power gate (OLED VCC control)
#define PIN_LED          NRF_GPIO(0, 15)
// P0.09 / P0.10: Dedicated NFC antenna pins. Do not map to standard GPIO.

// ============================ Reed Switch ============================
#define REED_ARM_CONFIRM_MS      2000UL   // Stable closure threshold to trigger ARMED state
#define REED_RELEASE_CONFIRM_MS  10000UL  // Stable open threshold to trigger RELEASED state

// ========================== Button / Display =========================
#define BUTTON_DEBOUNCE_MS    30UL
#define BUTTON_LONG_PRESS_MS  5000UL      // Medic maintenance / reset hold duration
#define DISPLAY_ON_MS         4000UL      // On-demand blackout HUD duration
#define OLED_I2C_ADDR         0x3C
#define OLED_WIDTH            128
#define OLED_HEIGHT           64

// ======================= TCCC Triage Thresholds (sec) ================
// SAFE < ATTENTION <= ... < CRITICAL. TCCC strict occlusion ceiling: 2 hours.
#define TRIAGE_ATTENTION_SEC  (90UL * 60UL)
#define TRIAGE_CRITICAL_SEC   (120UL * 60UL)

// =============================== BLE =================================
#define BLE_COMPANY_ID         0xFFFF  // Development Company Identifier (Bluetooth SIG)
#define BLE_ADV_INTERVAL_MS    1000    // Non-connectable advertising period
#define BLE_TX_POWER_DBM       0       // Allowed: -40, -20, -16, -12, -8, -4, 0, +4, +8
#define BLE_PAYLOAD_UPDATE_MS  5000UL  // Payload refresh interval
#define BLE_DEVICE_NAME        "STQ"   // Short advertisement name (fits 31-byte limit)

// ============================== Battery ==============================
#define BATTERY_LOW_PCT        20
#define BATTERY_MEASURE_MS     60000UL

// ============================ Persistence ============================
#define PERSIST_INTERVAL_MS    60000UL  // Periodic flash backup period (brownout protection)

// ============================= Main Loop =============================
#define LOOP_IDLE_MS           100UL    // FreeRTOS tickless idle delay per iteration
