/**
 * @file nfc_tag.h
 * @brief Secondary backup telemetry channel using nRF52 internal NFCT peripheral (13.56 MHz).
 *
 * Emulates an NFC Forum Type 2 Tag with NDEF payload.
 * Allows zero-RF emissions reading by medic smartphone touch.
 */
#pragma once
#include <Arduino.h>
#include "../core/beacon_packet.h"

class NfcTag {
 public:
  bool begin();
  void update(const BeaconPacket::Payload& p);  // Update serialized NDEF record

 private:
  bool _ready = false;
};
