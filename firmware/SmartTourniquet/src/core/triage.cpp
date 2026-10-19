#include "triage.h"

namespace Triage {

TriageLevel fromElapsed(uint32_t elapsedSec, uint32_t attentionSec, uint32_t criticalSec) {
  if (elapsedSec >= criticalSec)  return TriageLevel::CRITICAL;
  if (elapsedSec >= attentionSec) return TriageLevel::ATTENTION;
  return TriageLevel::SAFE;
}

const char* label(TriageLevel level) {
  switch (level) {
    case TriageLevel::SAFE:      return "SAFE";
    case TriageLevel::ATTENTION: return "ATTENTION";
    case TriageLevel::CRITICAL:  return "CRITICAL";
  }
  return "?";
}

}  // namespace Triage
