#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "../../config.h"
#include "../core/triage.h"

struct StatusView {
  uint16_t    deviceId;
  uint32_t    elapsedSec;
  uint8_t     batteryPct;
  TriageLevel level;
  bool        released;
  bool        timeRestored;
  bool        lowBattery;
  bool        resetConfirmPending;
};

class OledDisplay {
 public:
  void begin();
  void show(const StatusView& v);
  void off();
  bool isOn() const { return _on; }

 private:
  bool powerUp();
  void powerDown();
  void drawBatteryIcon(int16_t x, int16_t y, uint8_t pct);

  Adafruit_SSD1306 _oled{OLED_WIDTH, OLED_HEIGHT, &Wire, -1};
  bool _on = false;
  bool _ready = false;
};
