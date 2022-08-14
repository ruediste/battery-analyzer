#include "config.h"

#if IS_FRAMEWORK_ARDUINO
#include <Arduino.h>

#include "display.h"
#include "menu.h"
#include "controller.h"
#include "input.h"

uint32_t next;
bool state = false;

void setup()
{
   Serial.begin(115200);
  // put your setup code here, to run once:
  controller::init();
  pinMode(PC13, OUTPUT);
  next = millis();

}

void loop()
{
  // put your main code here, to run repeatedly:
  controller::loop();
  if (millis() > next)
  {
    next = millis() + 500;
    state = !state;
    digitalWrite(PC13,state?HIGH: LOW);
    Serial.println(state);
  }
}
#endif