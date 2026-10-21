#include "persistence.h"
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

namespace {
const char* kStateFile = "/stq_state.bin";

// Retained across soft resets, watchdog resets, and brief brownouts without Flash wear.
__attribute__((section(".noinit"))) Snapshot s_ramSnapshot;
}  // namespace

uint32_t Persistence::computeCrc(const Snapshot& s) {
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&s);
  constexpr size_t len = offsetof(Snapshot, crc32);
  uint32_t crc = 0xFFFFFFFFUL;
  for (size_t i = 0; i < len; i++) {
    crc ^= bytes[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 1UL) {
        crc = (crc >> 1) ^ 0xEDB88320UL;
      } else {
        crc >>= 1;
      }
    }
  }
  return ~crc;
}

bool Persistence::isValid(const Snapshot& s) {
  return (s.magic == MAGIC) && (s.version == VERSION) && (s.crc32 == computeCrc(s));
}

bool Persistence::begin() {
  _fsReady = InternalFS.begin();
  return _fsReady;
}

bool Persistence::load(Snapshot& out) {
  const bool ramOk = isValid(s_ramSnapshot);

  Snapshot flashSnap{};
  bool flashOk = false;
  if (_fsReady) {
    File f(InternalFS);
    if (f.open(kStateFile, FILE_O_READ)) {
      if (f.read(&flashSnap, sizeof(flashSnap)) == sizeof(flashSnap)) {
        flashOk = isValid(flashSnap);
      }
      f.close();
    }
  }

  if (ramOk && flashOk) {
    out = (s_ramSnapshot.elapsedSec >= flashSnap.elapsedSec) ? s_ramSnapshot : flashSnap;
    return true;
  }
  if (ramOk) {
    out = s_ramSnapshot;
    return true;
  }
  if (flashOk) {
    out = flashSnap;
    return true;
  }
  return false;
}

void Persistence::cacheRam(uint32_t elapsedSec, uint8_t flags) {
  Snapshot s{};
  s.magic      = MAGIC;
  s.elapsedSec = elapsedSec;
  s.flags      = flags;
  s.version    = VERSION;
  s.reserved   = 0;
  s.crc32      = computeCrc(s);
  s_ramSnapshot = s;
}

void Persistence::save(uint32_t elapsedSec, uint8_t flags) {
  cacheRam(elapsedSec, flags);

  if (!_fsReady) return;
  if (elapsedSec == _lastFlashElapsed && flags == _lastFlashFlags) return;

  InternalFS.remove(kStateFile);
  File f(InternalFS);
  if (f.open(kStateFile, FILE_O_WRITE)) {
    f.write(reinterpret_cast<const uint8_t*>(&s_ramSnapshot), sizeof(s_ramSnapshot));
    f.close();
    _lastFlashElapsed = elapsedSec;
    _lastFlashFlags   = flags;
  }
}

void Persistence::clear() {
  memset(&s_ramSnapshot, 0, sizeof(s_ramSnapshot));
  _lastFlashElapsed = 0xFFFFFFFFUL;
  _lastFlashFlags   = 0xFFU;
  if (_fsReady) {
    InternalFS.remove(kStateFile);
  }
}
