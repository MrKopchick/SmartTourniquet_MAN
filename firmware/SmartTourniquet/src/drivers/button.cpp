#include "button.h"

void Button::begin(uint8_t pin, uint32_t debounceMs, uint32_t longPressMs) {
  _pin = pin;
  _debounceMs = debounceMs;
  _longMs = longPressMs;
  pinMode(_pin, INPUT_PULLUP);  // Pull-up draws zero current while open
  _lastRaw = digitalRead(_pin) == LOW;
  _changedAt = millis();

  // TODO: Implement external pin interrupt wake-up for instantaneous edge handling.
}

Button::Event Button::update() {
  const bool raw = digitalRead(_pin) == LOW;
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
  } else if (raw && _pressed && !_longFired && now - _pressedAt >= _longMs) {
    _longFired = true;
    return Event::LONG_PRESS;
  } else if (!raw && _pressed) {
    _pressed = false;
    if (!_longFired) return Event::SHORT_PRESS;
  }
  return Event::NONE;
}
