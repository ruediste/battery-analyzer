#ifndef DISPLAY_H
#define DISPLAY_H
#include "types.h"
#include "config.h"
namespace display{
    void init();

    void print(const char *str);
    size_t print(long n, int base);
    void clear();
    void setCursor(uint8_t x, uint8_t y);

#ifdef IS_FRAMEWORK_NATIVE
    void show();
    void hide();
#endif
}
#endif