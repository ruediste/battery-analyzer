#ifndef NUMBER_INPUT_H
#define NUMBER_INPUT_H

#include "types.h"
namespace numberInput
{
    struct Callbacks
    {
        void (*success)(uint32_t value) = [](uint32_t value) {};
        void (*cancel)() = []() {};
        void (*print)() = []() {};
        void (*valueChanged)(uint32_t value) = [](uint32_t value) {};
    };

    void enter(uint32_t initialValue, int digits, int fraction, void (*setup)(Callbacks &callbacks));
    bool active();
    void loop();

};
#endif