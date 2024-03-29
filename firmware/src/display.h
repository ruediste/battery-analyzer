#ifndef DISPLAY_H
#define DISPLAY_H
#include "types.h"
#include "config.h"

#if IS_FRAMEWORK_ARDUINO
#include <Arduino.h>
#endif

namespace display
{
    void init();

    size_t print(const char *str);
    size_t print(int n);
    size_t print(long n);
    size_t print(double n, int prec = 2);
    void print(const char ch);
    void clear();
    void setCursor(uint8_t x, uint8_t y);

#if IS_FRAMEWORK_ARDUINO
    size_t print(const __FlashStringHelper *ifsh);
#endif

}
#endif