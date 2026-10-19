#include "occlusion_timer.h"

void OcclusionTimer::start(uint32_t initialSec) {
  _accumSec = initialSec;
  _startMs = millis();
  _running = true;
}

void OcclusionTimer::freeze() {
  if (!_running) return;
  _accumSec = elapsedSec();
  _running = false;
}

void OcclusionTimer::resume() {
  if (_running) return;
  _startMs = millis();
  _running = true;
}

void OcclusionTimer::reset() {
  _running = false;
  _accumSec = 0;
}

uint32_t OcclusionTimer::elapsedSec() const {
  if (!_running) return _accumSec;
  // Unsigned subtraction safely handles 32-bit rollover (~49.7 days).
  return _accumSec + (millis() - _startMs) / 1000UL;
}
