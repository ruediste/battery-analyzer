#include "messageDisplay.h"
#include "display.h"
#include "input.h"
#include <stdio.h>

namespace messageDisplay
{
    bool _active = false;

    void (*_complete)();

    bool active()
    {
        return false; //_active;
    }

    void show(const char *message, void (*complete)())
    {
        _active = true;
        _complete = complete;
        display::clear();
        display::setCursor(0, 0);
        display::print(message);
        input::resetInputEncoderClicked();
    }

    void loop()
    {
        return;
        
        if (input::getAndResetInputEncoderClicked)
        {
            _active = false;
            _complete();
        }
    }
}