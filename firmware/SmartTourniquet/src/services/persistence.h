#pragma once
#include <Arduino.h>

// Dual-tier state persistence: no-init SRAM (1 Hz, zero Flash wear) + LittleFS Flash.
struct Snapshot {
  uint32_t magic;
  uint32_t elapsedSec;
  uint8_t  flags;
  uint8_t  version;
  uint16_t reserved;
  uint32_t crc32;  // IEEE 802.3 CRC-32 over preceding 12 bytes
};

class Persistence {
 public:
  bool begin();
  bool load(Snapshot& out);
  void cacheRam(uint32_t elapsedSec, uint8_t flags);
  void save(uint32_t elapsedSec, uint8_t flags);
  void clear();

  static constexpr uint32_t MAGIC   = 0x53545132UL;  // "STQ2"
  static constexpr uint8_t  VERSION = 1;

 private:
  static uint32_t computeCrc(const Snapshot& s);
  static bool     isValid(const Snapshot& s);

  bool     _fsReady = false;
  uint32_t _lastFlashElapsed = 0xFFFFFFFFUL;
  uint8_t  _lastFlashFlags   = 0xFFU;
};
