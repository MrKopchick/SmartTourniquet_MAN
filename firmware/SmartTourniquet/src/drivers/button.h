#pragma once
#include <Arduino.h>

class Button {
 public:
  enum class Event : uint8_t { NONE, SHORT_PRESS, LONG_PRESS };
  using WakeCallback = void (*)();

  void  begin(uint8_t pin, uint32_t debounceMs, uint32_t longPressMs, WakeCallback onWake = nullptr);
  Event update();
  bool  isPressed() const { return _pressed; }

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
