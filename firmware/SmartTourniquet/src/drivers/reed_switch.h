#pragma once
#include <Arduino.h>

class ReedSwitch {
 public:
  enum class Event : uint8_t { NONE, CLOSED, OPENED };

  void  begin(uint8_t pin, uint32_t closeConfirmMs, uint32_t openConfirmMs);
  Event update();
  bool  isClosedRaw() const;
  bool  isClosed() const { return _stableClosed; }
  bool  waitStableClosed(uint32_t confirmMs);

 private:
  uint8_t  _pin = 0;
  uint32_t _closeMs = 0;
  uint32_t _openMs = 0;
  bool     _stableClosed = false;
  bool     _lastRaw = false;
  uint32_t _changedAt = 0;
};
