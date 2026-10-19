#include "power_manager.h"
#include "../../config.h"

namespace PowerManager {

static uint32_t s_resetReason = 0;

void begin() {
  // Read RESETREAS prior to SoftDevice initialization.
  s_resetReason = NRF_POWER->RESETREAS;
  NRF_POWER->RESETREAS = 0xFFFFFFFF;  // Clear flags (write-1-to-clear)

  // TODO: Enable internal DC/DC converter if inductor components exist on board.
  // TODO: Ensure native USB peripheral is halted in production release builds.
}

void idle(uint32_t ms) {
  // delay() in Adafruit nRF52 core triggers FreeRTOS vTaskDelay (WFE sleep).
  delay(ms);
}

void enterSystemOff(uint8_t wakePin) {
  DBG("[PWR] Entering System OFF, wake pin: %u\n", wakePin);
  systemOff(wakePin, LOW);
  while (true) {}  // Unreachable
}

bool wokeFromSystemOff() {
  return (s_resetReason & POWER_RESETREAS_OFF_Msk) != 0;
}

uint32_t resetReason() {
  return s_resetReason;
}

uint16_t deviceId() {
  return (uint16_t)(NRF_FICR->DEVICEID[0] & 0xFFFF);
}

}  // namespace PowerManager
