#include "config.h"
#include "src/app/controller.h"

static Controller controller;

void setup() {
  controller.begin();  // Enters System OFF and does not return if unoccluded
}

void loop() {
  controller.loop();
}
