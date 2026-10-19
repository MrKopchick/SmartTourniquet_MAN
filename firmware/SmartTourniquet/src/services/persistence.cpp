#include "persistence.h"
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

static const char* kStateFile = "/stq_state.bin";

bool Persistence::begin() {
  return InternalFS.begin();
}

bool Persistence::load(Snapshot& out) {
  File f(InternalFS);
  if (!f.open(kStateFile, FILE_O_READ)) return false;
  const bool ok = f.read(&out, sizeof(out)) == sizeof(out) && out.magic == MAGIC;
  f.close();
  return ok;
}

void Persistence::save(uint32_t elapsedSec, uint8_t flags) {
  Snapshot s{};
  s.magic = MAGIC;
  s.elapsedSec = elapsedSec;
  s.flags = flags;

  // Replace file with latest snapshot
  InternalFS.remove(kStateFile);
  File f(InternalFS);
  if (f.open(kStateFile, FILE_O_WRITE)) {
    f.write(reinterpret_cast<const uint8_t*>(&s), sizeof(s));
    f.close();
  }
}

void Persistence::clear() {
  InternalFS.remove(kStateFile);
}
