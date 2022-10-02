#include "messageDisplay.h"
#include "display.h"
#include "input.h"
#include <stdio.h>

namespace messageDisplay
{
    bool _active = false;

    void (*_complete)();
    void (*_cancel)();

    int _selection;

    bool active()
    {
        return _active;
    }

    void updateSelectionDisplay(int newSelection)
    {
        if (_selection == 0)
        {
            display::setCursor(0, 3);
            display::print(F("  "));
        }
        else
        {
            display::setCursor(10, 3);
            display::print(F("      "));
        }
        _selection = newSelection;
        if (_selection == 0)
        {
            display::setCursor(0, 3);
            display::print(F("^^"));
        }
        else
        {
            display::setCursor(10, 3);
            display::print(F("^^^^^^"));
        }
    }

    void show(const char *message, void (*complete)(), void (*cancel)())
    {
        _active = true;
        _complete = complete;
        _cancel = cancel;
        _selection = cancel == NULL ? 0 : 1;

        display::clear();
        display::setCursor(0, 0);
        display::print(message);

        display::setCursor(0, 2);
        display::print(F("OK"));

        if (cancel != NULL)
        {
            display::setCursor(10, 2);
            display::print(F("CANCEL"));
        }

        updateSelectionDisplay(_selection);

        input::getAndResetInputEncoder();
        input::resetInputEncoderClicked();
    }

    void loop()
    {
        if (_active)
        {
            int move = input::getAndResetInputEncoder();
            if (move != 0)
            {
                if (_cancel == NULL)
                {
                    // NOP
                }
                else
                {
                    int newSelection = _selection + move;
                    if (newSelection < 0)
                        newSelection = 0;
                    if (newSelection > 1)
                        newSelection = 1;
                    updateSelectionDisplay(newSelection);
                }
            }
            if (input::getAndResetInputEncoderClicked())
            {
                _active = false;
                if (_selection == 0)
                    _complete();
                else
                    _cancel();
            }
        }
    }
}