/**
 * @file power_manager.h
 * @brief Power state and hardware power optimization manager for nRF52840.
 *
 * State budget:
 * - System OFF: ~0.4 uA (Warehouse / IFAK storage before occlusion).
 * - System ON idle: ~2-3 uA (Active occlusion counting).
 */
#pragma once
#include <Arduino.h>

namespace PowerManager {

void     begin();                                // Read reset reason and configure power rails
void     idle(uint32_t ms);                      // System ON tickless sleep via WFE
[[noreturn]] void enterSystemOff(uint8_t wakePin);  // Enter ultra-low-power deep sleep
bool     wokeFromSystemOff();                    // True if booted via wake pin trigger
uint32_t resetReason();                          // Raw NRF_POWER->RESETREAS
uint16_t deviceId();                             // Truncated unique ID from FICR

}  // namespace PowerManager
