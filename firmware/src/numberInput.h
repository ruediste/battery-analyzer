#ifndef NUMBER_INPUT_H
#define NUMBER_INPUT_H

#include "types.h"
namespace numberInput{
    void enter(uint32_t initialValue,int digits, int fraction, void (*success)(uint32_t value), void (*cancel)());
    bool active();
    void loop();
}
#endif