/**
 * @file battery.h
 * @brief Battery voltage measurement and state-of-charge estimator.
 * Measures VDD directly using nRF52 internal SAADC.
 */
#pragma once
#include <Arduino.h>

class Battery {
 public:
  void     begin();
  void     measure();                       // Refresh sampled reading
  uint16_t millivolts() const { return _mv; }
  uint8_t  percent() const { return _pct; }
  bool     isLow(uint8_t thresholdPct) const { return _pct <= thresholdPct; }

 private:
  uint16_t readVddMillivolts();
  static uint8_t mvToPercent(uint16_t mv);

  uint16_t _mv = 3000;
  uint8_t  _pct = 100;
};
