#include "reed_switch.h"
#include "../../config.h"

void ReedSwitch::begin(uint8_t pin, uint32_t closeConfirmMs, uint32_t openConfirmMs) {
  _pin = pin;
  _closeMs = closeConfirmMs;
  _openMs = openConfirmMs;

  pinMode(_pin, INPUT);

  _lastRaw = isClosedRaw();
  _stableClosed = _lastRaw;
  _changedAt = millis();
}

bool ReedSwitch::isClosedRaw() const {
#if REED_EXTERNAL_PULLUP
  return digitalRead(_pin) == LOW;
#else
  // Pulse internal ~13 kOhm pull-up for 5 us to avoid ~230 uA continuous drain when closed.
  pinMode(_pin, INPUT_PULLUP);
  delayMicroseconds(5);
  const bool closed = (digitalRead(_pin) == LOW);
  pinMode(_pin, INPUT);
  return closed;
#endif
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
    if (!isClosedRaw()) return false;
    delay(50);
  }
  _stableClosed = true;
  _lastRaw = true;
  _changedAt = millis();
  return true;
}
