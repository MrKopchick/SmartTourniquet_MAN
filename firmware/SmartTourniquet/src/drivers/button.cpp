#include "button.h"

namespace {
Button::WakeCallback s_wakeCallback = nullptr;

void buttonIsr() {
  if (s_wakeCallback) {
    s_wakeCallback();
  }
}
}  // namespace

void Button::begin(uint8_t pin, uint32_t debounceMs, uint32_t longPressMs, WakeCallback onWake) {
  _pin = pin;
  _debounceMs = debounceMs;
  _longMs = longPressMs;
  s_wakeCallback = onWake;

  pinMode(_pin, INPUT_PULLUP);
  _lastRaw = (digitalRead(_pin) == LOW);
  _changedAt = millis();

  attachInterrupt(digitalPinToInterrupt(_pin), buttonIsr, CHANGE);
}

Button::Event Button::update() {
  const bool raw = (digitalRead(_pin) == LOW);
  const uint32_t now = millis();

  if (raw != _lastRaw) {
    _lastRaw = raw;
    _changedAt = now;
  }
  if (now - _changedAt < _debounceMs) return Event::NONE;

  if (raw && !_pressed) {
    _pressed = true;
    _longFired = false;
    _pressedAt = now;
  } else if (raw && _pressed && !_longFired && (now - _pressedAt >= _longMs)) {
    _longFired = true;
    return Event::LONG_PRESS;
  } else if (!raw && _pressed) {
    _pressed = false;
    if (!_longFired) return Event::SHORT_PRESS;
  }
  return Event::NONE;
}
