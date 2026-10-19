/**
 * @file ble_beacon.h
 * @brief BLE 5.0 non-connectable undirected advertising beacon.
 * Broadcasts telemetry to any listening medic receiver without pairing.
 */
#pragma once
#include <Arduino.h>
#include "../core/beacon_packet.h"

class BleBeacon {
 public:
  bool begin();
  void update(const BeaconPacket::Payload& p);  // Encode and update advertising payload
  void stop();

 private:
  bool _started = false;
};
