#pragma once
#include <Arduino.h>

namespace PowerManager {

void     begin();
void     enableDcDc();
void     feedWatchdog();
void     idle(uint32_t maxMs);
void     notifyWakeFromISR();
[[noreturn]] void enterSystemOff(uint8_t wakePin);
bool     wokeFromSystemOff();
uint32_t resetReason();
uint16_t deviceId();

}  // namespace PowerManager
