#include "config.h"

#if IS_FRAMEWORK_ARDUINO
#include <Arduino.h>

#include "display.h"
#include <LiquidCrystal_I2C.h>

// STM32: D14/D15 (PB8/PB9)

LiquidCrystal_I2C lcd(0x27, 20, 4);

namespace display
{

  void init()
  {
    lcd.init();
    lcd.backlight();
  }

  size_t print(double n) { return lcd.print(n); }
  size_t print(const char *str) { return lcd.print(str); }
  size_t print(const __FlashStringHelper *ifsh) { return lcd.print(ifsh); }

  size_t print(int n) { return lcd.print(n); }
  size_t print(long n) { return lcd.print(n); }

  void print(const char ch) { lcd.print(ch); }
  void clear() { lcd.clear(); }
  void setCursor(uint8_t x, uint8_t y) { lcd.setCursor(x, y); }
}
#endif