#pragma once
#include <Arduino.h>

// Hardware RTC stops in System OFF mode; continuous tracking requires System ON idle.
class OcclusionTimer {
 public:
  void     start(uint32_t initialSec = 0);
  void     freeze();
  void     resume();
  void     reset();
  bool     isRunning() const { return _running; }
  uint32_t elapsedSec() const;
  uint64_t elapsedMs() const;

 private:
  bool     _running = false;
  uint32_t _lastSampleMs = 0;
  uint64_t _accumMs = 0;
};
