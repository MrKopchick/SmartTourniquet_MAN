#pragma once
#include <stdint.h>

enum class TriageLevel : uint8_t {
  SAFE      = 0,
  ATTENTION = 1,
  CRITICAL  = 2,
};

namespace Triage {

TriageLevel fromElapsed(uint32_t elapsedSec, uint32_t attentionSec, uint32_t criticalSec);
uint32_t    remainingToCriticalSec(uint32_t elapsedSec, uint32_t criticalSec);
uint16_t    progressPermille(uint32_t elapsedSec, uint32_t criticalSec);
const char* label(TriageLevel level);

}