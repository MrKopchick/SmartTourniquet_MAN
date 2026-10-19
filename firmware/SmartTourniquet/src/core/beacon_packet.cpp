#include "beacon_packet.h"

namespace BeaconPacket {

static inline void putU16(uint8_t* p, uint16_t v) {
  p[0] = v & 0xFF;
  p[1] = (v >> 8) & 0xFF;
}

static inline void putU32(uint8_t* p, uint32_t v) {
  for (uint8_t i = 0; i < 4; i++) p[i] = (v >> (8 * i)) & 0xFF;
}

uint8_t checksum(const uint8_t* data, uint8_t len) {
  uint8_t x = 0;
  for (uint8_t i = 0; i < len; i++) x ^= data[i];
  return x;
}

void encode(const Payload& p, uint8_t out[LENGTH]) {
  putU16(&out[0], p.companyId);
  out[2] = PROTOCOL_VERSION;
  putU16(&out[3], p.deviceId);
  putU32(&out[5], p.elapsedSec);
  out[9]  = p.batteryPct;
  out[10] = p.flags;
  out[11] = checksum(out, LENGTH - 1);
  // TODO: Add cryptographic MAC (e.g., truncated AES-CMAC) to prevent spoofing.
}

}  // namespace BeaconPacket
