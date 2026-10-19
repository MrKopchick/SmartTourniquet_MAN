#include "reed_switch.h"

void ReedSwitch::begin(uint8_t pin, uint32_t closeConfirmMs, uint32_t openConfirmMs) {
  _pin = pin;
  _closeMs = closeConfirmMs;
  _openMs = openConfirmMs;

  // Note: Internal ~13k pull-up draws ~230 uA when contact is closed.
  // Hardware optimization options:
  // (a) External 1-2 MOhm pull-up resistor.
  // (b) Sampled pull-up duty cycling during sleep intervals.
  pinMode(_pin, INPUT_PULLUP);

  _lastRaw = isClosedRaw();
  _stableClosed = _lastRaw;
  _changedAt = millis();
}

bool ReedSwitch::isClosedRaw() const {
  return digitalRead(_pin) == LOW;  // Normally open contact connects to GND
}

ReedSwitch::Event ReedSwitch::update() {
  const bool raw = isClosedRaw();
  const uint32_t now = millis();

  if (raw != _lastRaw) {
    _lastRaw = raw;
    _changedAt = now;
  }

  if (raw != _stableClosed) {
    const uint32_t need = raw ? _closeMs : _openMs;
    if (now - _changedAt >= need) {
      _stableClosed = raw;
      return raw ? Event::CLOSED : Event::OPENED;
    }
  }
  return Event::NONE;
}

bool ReedSwitch::waitStableClosed(uint32_t confirmMs) {
  const uint32_t start = millis();
  while (millis() - start < confirmMs) {
    if (!isClosedRaw()) return false;  // False positive reject (impact or stray magnetic field)
    delay(50);
  }
  _stableClosed = true;
  _lastRaw = true;
  _changedAt = millis();
  return true;
}
