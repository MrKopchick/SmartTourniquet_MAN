#include "battery.h"

void Battery::begin() {
  measure();
}

void Battery::measure() {
  _mv = readVddMillivolts();
  _pct = mvToPercent(_mv);
}

uint16_t Battery::readVddMillivolts() {
  // TODO: Configure SAADC (PSELP = VDD, reference = 0.6V, gain = 1/6, 12-bit).
  // Note: Sample immediately post-BLE TX to evaluate internal cell impedance under load.
  return 3000;  // Stub value
}

uint8_t Battery::mvToPercent(uint16_t mv) {
  // Piecewise linear discharge approximation for CR2032 under low continuous drain.
  struct Point { uint16_t mv; uint8_t pct; };
  static const Point curve[] = {
    {3000, 100}, {2900, 80}, {2800, 60}, {2700, 40}, {2600, 20}, {2400, 0},
  };
  const uint8_t n = sizeof(curve) / sizeof(curve[0]);

  if (mv >= curve[0].mv) return 100;
  if (mv <= curve[n - 1].mv) return 0;
  for (uint8_t i = 1; i < n; i++) {
    if (mv >= curve[i].mv) {
      const Point& hi = curve[i - 1];
      const Point& lo = curve[i];
      return lo.pct + (uint32_t)(mv - lo.mv) * (hi.pct - lo.pct) / (hi.mv - lo.mv);
    }
  }
  return 0;
}
