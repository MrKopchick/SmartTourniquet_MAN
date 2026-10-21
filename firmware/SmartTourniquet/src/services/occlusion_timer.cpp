#include "occlusion_timer.h"
#include "../../config.h"

void OcclusionTimer::start(uint32_t initialSec) {
  _accumMs = static_cast<uint64_t>(initialSec) * 1000ULL;
  _lastSampleMs = millis();
  _running = true;
}

void OcclusionTimer::freeze() {
  if (!_running) return;
  _accumMs = elapsedMs();
  _running = false;
}

void OcclusionTimer::resume() {
  if (_running) return;
  _lastSampleMs = millis();
  _running = true;
}

void OcclusionTimer::reset() {
  _running = false;
  _accumMs = 0;
  _lastSampleMs = millis();
}

uint64_t OcclusionTimer::elapsedMs() const {
  if (!_running) return _accumMs;
  // Unsigned 32-bit subtraction safely handles millis() wrap-around (~49.7 days).
  const uint32_t deltaMs = millis() - _lastSampleMs;
  return _accumMs + static_cast<uint64_t>(deltaMs) * TIME_SCALE_FACTOR;
}

uint32_t OcclusionTimer::elapsedSec() const {
  return static_cast<uint32_t>(elapsedMs() / 1000ULL);
}
