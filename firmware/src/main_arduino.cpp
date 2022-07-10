#include "config.h"

#if IS_FRAMEWORK_ARDUINO
#include <Arduino.h>

#include "display.h"
#include "menu.h"
#include "controller.h"
#include "input.h"

void setup() {
  // put your setup code here, to run once:
  controller::init();
}

void loop() {
  // put your main code here, to run repeatedly:
  controller::loop();
}
#endif