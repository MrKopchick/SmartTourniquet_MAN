/**
 * @file SmartTourniquet.ino
 * @brief Firmware entry point for the SmartTourniquet clip.
 * MCU: Nordic nRF52840 (Adafruit nRF52 core).
 *
 * Execution delegates directly to src/app/controller.
 */
#include "config.h"
#include "src/app/controller.h"

static Controller controller;

void setup() {
  controller.begin();  // May not return if unoccluded; transitions to System OFF.
}

void loop() {
  controller.loop();
}
