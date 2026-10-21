#include "battery.h"
#include "../../config.h"

void Battery::begin() {
  // SAADC config: 0.6V internal ref, 1/6 gain => 3600 mV full scale, 12-bit.
  analogReference(AR_INTERNAL);
  analogReadResolution(12);
  analogSampleTime(10);
  analogOversampling(4);
  analogCalibrateOffset();

  _mv = readVddMillivolts();
  _pct = mvToPercent(_mv);
  _initialized = true;
}

void Battery::measure() {
  const uint16_t sampleMv = readVddMillivolts();
  if (!_initialized) {
    _mv = sampleMv;
    _initialized = true;
  } else {
    _mv = static_cast<uint16_t>(
        (static_cast<uint32_t>(sampleMv) * BATTERY_EMA_ALPHA_NUM +
         static_cast<uint32_t>(_mv) * (BATTERY_EMA_ALPHA_DEN - BATTERY_EMA_ALPHA_NUM)) /
        BATTERY_EMA_ALPHA_DEN);
  }
  _pct = mvToPercent(_mv);
}

uint16_t Battery::readVddMillivolts() {
  analogReference(AR_INTERNAL);
  analogReadResolution(12);
  const uint32_t raw = analogReadVDD();
  return static_cast<uint16_t>((raw * 3600UL + 2048UL) / 4096UL);
}

uint8_t Battery::mvToPercent(uint16_t mv) {
  struct Point {
    uint16_t mv;
    uint8_t  pct;
  };
  static const Point curve[] = {
    {3000, 100},
    {2900, 80},
    {2800, 60},
    {2700, 40},
    {2600, 20},
    {2400, 0},
  };
  constexpr uint8_t n = sizeof(curve) / sizeof(curve[0]);

  if (mv >= curve[0].mv) return 100;
  if (mv <= curve[n - 1].mv) return 0;
  for (uint8_t i = 1; i < n; i++) {
    if (mv >= curve[i].mv) {
      const Point& hi = curve[i - 1];
      const Point& lo = curve[i];
      return static_cast<uint8_t>(
          lo.pct + static_cast<uint32_t>(mv - lo.mv) * (hi.pct - lo.pct) / (hi.mv - lo.mv));
    }
  }
  return 0;
}
