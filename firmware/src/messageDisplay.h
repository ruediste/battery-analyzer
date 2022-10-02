#ifndef MESSAGE_DISPLAY_H
#define MESSAGE_DISPLAY_H

#include "types.h"
namespace messageDisplay
{
    bool active();
    void show(const char *message, void (*complete)(), void (*cancel)() = NULL);
    void loop();
};
#endif