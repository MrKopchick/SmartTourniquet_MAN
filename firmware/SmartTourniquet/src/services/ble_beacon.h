#pragma once
#include <Arduino.h>
#include "../core/beacon_packet.h"
#include "../core/triage.h"

class BleBeacon {
 public:
  bool begin();
  void update(const BeaconPacket::Payload& p, TriageLevel level);
  void stop();
  bool isRunning() const { return _started; }

 private:
  static uint16_t intervalUnitsFor(TriageLevel level);

  bool     _started = false;
  uint16_t _currentIntervalUnits = 0;
};
