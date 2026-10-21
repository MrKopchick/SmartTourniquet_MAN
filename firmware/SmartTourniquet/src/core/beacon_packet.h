#pragma once
#include <stdint.h>

// Wire format (12B LE): [0..1] companyId, [2] ver, [3..4] deviceId,
// [5..8] elapsedSec, [9] batteryPct, [10] flags, [11] xor(0..10).
namespace BeaconPacket {

constexpr uint8_t LENGTH           = 12;
constexpr uint8_t PROTOCOL_VERSION = 1;

enum Flags : uint8_t {
  FLAG_ARMED         = 1 << 0,
  FLAG_ATTENTION     = 1 << 1,
  FLAG_CRITICAL      = 1 << 2,
  FLAG_LOW_BATTERY   = 1 << 3,
  FLAG_RELEASED      = 1 << 4,
  FLAG_TIME_RESTORED = 1 << 5,
};

struct Payload {
  uint16_t companyId;
  uint16_t deviceId;
  uint32_t elapsedSec;
  uint8_t  batteryPct;
  uint8_t  flags;
};

void    encode(const Payload& p, uint8_t out[LENGTH]);
bool    decode(const uint8_t in[LENGTH], Payload& out);
uint8_t checksum(const uint8_t* data, uint8_t len);

}
