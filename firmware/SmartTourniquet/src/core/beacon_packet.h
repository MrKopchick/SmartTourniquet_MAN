/**
 * @file beacon_packet.h
 * @brief BLE Manufacturer Specific Data packet serialization.
 * Reference: docs/protocol.md.
 *
 * Binary layout (12 bytes, little-endian):
 *   [0..1]  Company ID
 *   [2]     Protocol version
 *   [3..4]  Device ID
 *   [5..8]  Elapsed occlusion time (seconds, uint32)
 *   [9]     Battery capacity (%)
 *   [10]    State flags
 *   [11]    Checksum (XOR of bytes 0..10)
 */
#pragma once
#include <stdint.h>

namespace BeaconPacket {

constexpr uint8_t LENGTH           = 12;
constexpr uint8_t PROTOCOL_VERSION = 1;

enum Flags : uint8_t {
  FLAG_ARMED         = 1 << 0,  // Tourniquet occluded, timer active
  FLAG_ATTENTION     = 1 << 1,  // Threshold >= 90 min
  FLAG_CRITICAL      = 1 << 2,  // Threshold >= 120 min (TCCC maximum)
  FLAG_LOW_BATTERY   = 1 << 3,  // Battery level below threshold
  FLAG_RELEASED      = 1 << 4,  // Tourniquet loosened/removed, time frozen
  FLAG_TIME_RESTORED = 1 << 5,  // State recovered from Flash after reset
  // Bits 6..7: Reserved
};

struct Payload {
  uint16_t companyId;
  uint16_t deviceId;
  uint32_t elapsedSec;
  uint8_t  batteryPct;
  uint8_t  flags;
};

/**
 * @brief Encodes payload struct into raw 12-byte buffer.
 */
void encode(const Payload& p, uint8_t out[LENGTH]);

/**
 * @brief Calculates XOR checksum across len bytes.
 */
uint8_t checksum(const uint8_t* data, uint8_t len);

}  // namespace BeaconPacket
