/**
 * @file occlusion_timer.h
 * @brief Occlusion duration tracking backed by hardware low-frequency RTC.
 *
 * Uses RTC1 clocked via LFXO (32.768 kHz external crystal, +/-20 ppm stability).
 * Immune to EW and RF jamming.
 *
 * Note: Hardware RTC stops in System OFF mode; continuous tracking operates
 * under System ON idle (~2-3 uA current draw).
 */
#pragma once
#include <Arduino.h>

class OcclusionTimer {
 public:
  void     start(uint32_t initialSec = 0);  // Non-zero parameter restores state after reboot
  void     freeze();                        // Halts and preserves final occlusion duration
  void     resume();                        // Resumes accumulation upon re-tightening
  void     reset();
  bool     isRunning() const { return _running; }
  uint32_t elapsedSec() const;

 private:
  bool     _running = false;
  uint32_t _startMs = 0;
  uint32_t _accumSec = 0;  // Preserved elapsed seconds prior to freeze or restored from Flash
};

// Ensure target variant.h initializes USE_LFXO rather than uncalibrated USE_LFRC.
