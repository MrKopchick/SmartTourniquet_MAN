/**
 * @file button.h
 * @brief Push-button driver handling short and long press events.
 *   SHORT_PRESS: Displays on-demand tactical HUD on OLED (3-5 seconds).
 *   LONG_PRESS: Maintenance and operational state reset after release.
 */
#pragma once
#include <Arduino.h>

class Button {
 public:
  enum class Event : uint8_t { NONE, SHORT_PRESS, LONG_PRESS };

  void  begin(uint8_t pin, uint32_t debounceMs, uint32_t longPressMs);
  Event update();

 private:
  uint8_t  _pin = 0;
  uint32_t _debounceMs = 0;
  uint32_t _longMs = 0;
  bool     _lastRaw = false;
  bool     _pressed = false;
  bool     _longFired = false;
  uint32_t _changedAt = 0;
  uint32_t _pressedAt = 0;
};
