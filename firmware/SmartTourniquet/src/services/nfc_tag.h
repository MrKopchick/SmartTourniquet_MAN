#pragma once
#include <Arduino.h>
#include "../core/beacon_packet.h"

// ISO/IEC 14443-A / NFC Forum Type 2 Tag emulation via internal nRF52840 NFCT (13.56 MHz).
class NfcTag {
 public:
  bool begin();
  void update(const BeaconPacket::Payload& p);
  void stop();
  bool isReady() const { return _ready; }

 private:
  bool _ready = false;
};
