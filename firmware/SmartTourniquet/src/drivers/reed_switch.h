/**
 * @file reed_switch.h
 * @brief Reed switch sensor driver with long-duration debounce filtering.
 *
 * Mechanical movement and transport vibration must not inadvertently reset
 * or prematurely stop the occlusion timer.
 */
#pragma once
#include <Arduino.h>

class ReedSwitch {
 public:
  enum class Event : uint8_t { NONE, CLOSED, OPENED };

  void  begin(uint8_t pin, uint32_t closeConfirmMs, uint32_t openConfirmMs);
  Event update();                              // Call periodically inside main loop
  bool  isClosedRaw() const;                   // Direct instantaneous pin state
  bool  isClosed() const { return _stableClosed; }
  bool  waitStableClosed(uint32_t confirmMs);  // Blocking validation helper (used in setup)

 private:
  uint8_t  _pin = 0;
  uint32_t _closeMs = 0;
  uint32_t _openMs = 0;
  bool     _stableClosed = false;
  bool     _lastRaw = false;
  uint32_t _changedAt = 0;
};
