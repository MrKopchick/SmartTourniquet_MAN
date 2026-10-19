/**
 * @file triage.h
 * @brief Occlusion duration evaluation based on TCCC guidelines.
 * Pure logic module independent of hardware runtime.
 */
#pragma once
#include <stdint.h>

enum class TriageLevel : uint8_t {
  SAFE      = 0,  // < TRIAGE_ATTENTION_SEC
  ATTENTION = 1,  // Approaching critical threshold
  CRITICAL  = 2,  // >= 2 hours: high risk of ischemic nerve/tissue damage
};

namespace Triage {

/**
 * @brief Evaluates triage category by total elapsed seconds.
 */
TriageLevel fromElapsed(uint32_t elapsedSec, uint32_t attentionSec, uint32_t criticalSec);

/**
 * @brief Returns status text label for UI/display.
 */
const char* label(TriageLevel level);

}  // namespace Triage
