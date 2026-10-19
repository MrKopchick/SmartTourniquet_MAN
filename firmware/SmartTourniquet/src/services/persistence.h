/**
 * @file persistence.h
 * @brief Flash state persistence using Internal LittleFS.
 * Preserves elapsed occlusion timing across brownout resets.
 */
#pragma once
#include <Arduino.h>

struct Snapshot {
  uint32_t magic;       // Identification magic header
  uint32_t elapsedSec;  // Recorded elapsed seconds
  uint8_t  flags;       // State flags at time of write
  uint8_t  reserved[3];
};

class Persistence {
 public:
  bool begin();
  bool load(Snapshot& out);
  void save(uint32_t elapsedSec, uint8_t flags);
  void clear();

  static constexpr uint32_t MAGIC = 0x53545131;  // "STQ1"
};
