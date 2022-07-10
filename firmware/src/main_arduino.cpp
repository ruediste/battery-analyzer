#include "config.h"

#if IS_FRAMEWORK_ARDUINO
#include <Arduino.h>

#include "display.h"
void setup() {
  // put your setup code here, to run once:
  display::init();
  display::print("Hello World");
}

void loop() {
  // put your main code here, to run repeatedly:
}
#endif