#include "nfc_tag.h"
#include "../../config.h"

bool NfcTag::begin() {
  // TODO: Initialize NFCT peripheral.
  // Prerequisites:
  // - Configure UICR->NFCPINS to ensure pins P0.09 and P0.10 operate in NFC mode.
  // - Connect matched antenna coil tuned for 13.56 MHz resonance.
  _ready = false;
  return _ready;
}

void NfcTag::update(const BeaconPacket::Payload& p) {
  if (!_ready) return;
  // TODO: Generate NDEF record:
  // - Text Record: "STQ 0001 | T+01:23:45 | CRITICAL | BAT 87%"
  // - Optional binary record for direct app decoding.
  (void)p;
}
