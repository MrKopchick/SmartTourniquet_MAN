#include "beacon_packet.h"

namespace BeaconPacket {

static inline void putU16(uint8_t* p, uint16_t v) {
  p[0] = static_cast<uint8_t>(v & 0xFFU);
  p[1] = static_cast<uint8_t>((v >> 8) & 0xFFU);
}

static inline uint16_t getU16(const uint8_t* p) {
  return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

static inline void putU32(uint8_t* p, uint32_t v) {
  for (uint8_t i = 0; i < 4; i++) {
    p[i] = static_cast<uint8_t>((v >> (8U * i)) & 0xFFU);
  }
}

static inline uint32_t getU32(const uint8_t* p) {
  return static_cast<uint32_t>(p[0]) |
         (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) |
         (static_cast<uint32_t>(p[3]) << 24);
}

uint8_t checksum(const uint8_t* data, uint8_t len) {
  uint8_t x = 0;
  for (uint8_t i = 0; i < len; i++) {
    x ^= data[i];
  }
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
}

bool decode(const uint8_t in[LENGTH], Payload& out) {
  if (!in) return false;
  if (in[2] != PROTOCOL_VERSION) return false;
  if (checksum(in, LENGTH - 1) != in[11]) return false;

  out.companyId  = getU16(&in[0]);
  out.deviceId   = getU16(&in[3]);
  out.elapsedSec = getU32(&in[5]);
  out.batteryPct = in[9];
  out.flags      = in[10];
  return true;
}

}
