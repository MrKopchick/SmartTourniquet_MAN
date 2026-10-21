#include "triage.h"

namespace Triage {

TriageLevel fromElapsed(uint32_t elapsedSec, uint32_t attentionSec, uint32_t criticalSec) {
  if (elapsedSec >= criticalSec)  return TriageLevel::CRITICAL;
  if (elapsedSec >= attentionSec) return TriageLevel::ATTENTION;
  return TriageLevel::SAFE;
}

uint32_t remainingToCriticalSec(uint32_t elapsedSec, uint32_t criticalSec) {
  return (elapsedSec >= criticalSec) ? 0UL : (criticalSec - elapsedSec);
}

uint16_t progressPermille(uint32_t elapsedSec, uint32_t criticalSec) {
  if (criticalSec == 0 || elapsedSec >= criticalSec) return 1000U;
  return static_cast<uint16_t>((static_cast<uint64_t>(elapsedSec) * 1000ULL) / criticalSec);
}

const char* label(TriageLevel level) {
  switch (level) {
    case TriageLevel::SAFE:      return "SAFE";
    case TriageLevel::ATTENTION: return "ATTENTION";
    case TriageLevel::CRITICAL:  return "CRITICAL";
  }
  return "UNKNOWN";
}

}
