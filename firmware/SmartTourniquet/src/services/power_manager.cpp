#include "power_manager.h"
#include <nrf_soc.h>
#include "rtos.h"
#include "../../config.h"

namespace PowerManager {

static uint32_t          s_resetReason = 0;
static SemaphoreHandle_t s_wakeSem     = nullptr;

void begin() {
  // readResetReason() retrieves NRF_POWER->RESETREAS cached and cleared by Adafruit core init().
  s_resetReason = readResetReason();

  if (!s_wakeSem) {
    s_wakeSem = xSemaphoreCreateBinary();
  }

  NRF_POWER->DCDCEN = 1;

  if ((NRF_WDT->RUNSTATUS & WDT_RUNSTATUS_RUNSTATUS_Msk) == 0) {
    NRF_WDT->CONFIG =
        (WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos) |
        (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos);
    NRF_WDT->CRV         = (WDT_TIMEOUT_SEC * 32768UL) - 1UL;
    NRF_WDT->RREN        = WDT_RREN_RR0_Msk;
    NRF_WDT->TASKS_START = 1;
  }
  feedWatchdog();
}

void enableDcDc() {
  uint8_t sd_en = 0;
  (void)sd_softdevice_is_enabled(&sd_en);
  if (sd_en) {
    (void)sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
  } else {
    NRF_POWER->DCDCEN = 1;
  }
}

void feedWatchdog() {
  NRF_WDT->RR[0] = WDT_RR_RR_Reload;
}

void idle(uint32_t maxMs) {
  if (s_wakeSem) {
    // Blocks in FreeRTOS tickless idle (WFE sleep) until timeout or GPIO ISR notification.
    (void)xSemaphoreTake(s_wakeSem, pdMS_TO_TICKS(maxMs));
  } else {
    delay(maxMs);
  }
}

void notifyWakeFromISR() {
  if (!s_wakeSem) return;
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(s_wakeSem, &higherPriorityTaskWoken);
  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void enterSystemOff(uint8_t wakePin) {
  const uint8_t resolvedPin = resolveArduinoPin(wakePin);
  DBG("[PWR] Entering System OFF, wake GPIO %u (Arduino pin %u)\n", wakePin, resolvedPin);

  // Wait for reed switch to open before arming SENSE_LOW wake to prevent immediate wake loop.
  pinMode(resolvedPin, INPUT_PULLUP);
  delayMicroseconds(10);
  while (digitalRead(resolvedPin) == LOW) {
    feedWatchdog();
    delay(50);
  }

  systemOff(resolvedPin, LOW);
  while (true) {}
}

bool wokeFromSystemOff() {
  return (s_resetReason & POWER_RESETREAS_OFF_Msk) != 0;
}

uint32_t resetReason() {
  return s_resetReason;
}

uint16_t deviceId() {
  return static_cast<uint16_t>(NRF_FICR->DEVICEID[0] & 0xFFFFU);
}

}  // namespace PowerManager
