/**
 * @file oled_display.h
 * @brief SSD1306 0.96" I2C OLED display driver with physical power isolation.
 * Power to display module is fully gated via PIN_EXT_POWER to eliminate sleep quiescent current.
 */
#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "../../config.h"
#include "../core/triage.h"

/**
 * @brief View model for OLED rendering.
 */
struct StatusView {
  uint16_t    deviceId;
  uint32_t    elapsedSec;
  uint8_t     batteryPct;
  TriageLevel level;
  bool        released;
};

class OledDisplay {
 public:
  void begin();
  void show(const StatusView& v);  // Power up, initialize, dim, and draw status
  void off();                      // Clear and power down module
  bool isOn() const { return _on; }

 private:
  void powerUp();
  void powerDown();

  Adafruit_SSD1306 _oled{OLED_WIDTH, OLED_HEIGHT, &Wire, -1};
  bool _on = false;
};
