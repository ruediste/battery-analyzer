#include "numberInput.h"
#include "display.h"
#include "input.h"
#include <stdio.h>

namespace numberInput
{

    bool isActive;
    int digits;
    int fraction;
    uint32_t value;
    int cursor = 0;
    bool editing;
    void (*success)(uint32_t value);
    void (*cancel)();

    int cursorColumn()
    {
        if (cursor >= digits)
            return cursor + 1;
        else
            return cursor;
    }

    bool cursorAtDone()
    {
        return cursor == digits + fraction;
    }

    bool cursorAtCancel()
    {
        return cursor == digits + fraction + 1;
    }

    int cursorDoneColumn()
    {
        return digits + fraction + 2;
    }

    int cursorCancelColumn()
    {
        return digits + fraction + 5;
    }

    void moveCursor(int newPos)
    {
        if (cursorAtDone())
        {
            display::setCursor(cursorDoneColumn(), 1);
            display::print("  ");
        }
        else if (cursorAtCancel())
        {
            display::setCursor(cursorCancelColumn(), 1);
            display::print("  ");
        }
        else
        {
            display::setCursor(cursorColumn(), 1);
            display::print(' ');
        }
        cursor = newPos;
        if (cursorAtDone())
        {
            display::setCursor(cursorDoneColumn(), 1);
            display::print("^^");
        }
        else if (cursorAtCancel())
        {
            display::setCursor(cursorCancelColumn(), 1);
            display::print("^^");
        }
        else
        {
            display::setCursor(cursorColumn(), 1);
            display::print('^');
        }
    }

    void printValue()
    {
        uint32_t tmp = value;
        for (int i = 0; i < fraction; i++)
        {
            display::setCursor(digits + fraction - i, 0);
            display::print((char)('0' + (tmp % 10)));
            tmp /= 10;
        }
        display::setCursor(digits, 0);
        display::print('.');
        for (int i = 0; i < digits; i++)
        {
            display::setCursor(digits - 1 - i, 0);
            display::print((char)('0' + (tmp % 10)));
            tmp /= 10;
        }
    }

    void enter(uint32_t initialValue, int digits, int fraction, void (*success)(uint32_t value), void (*cancel)())
    {
        numberInput::value = initialValue;
        numberInput::digits = digits;
        numberInput::fraction = fraction;
        numberInput::success = success;
        numberInput::cancel = cancel;

        display::clear();

        printValue();

        cursor = digits + fraction;
        isActive = true;
        editing = false;

        display::setCursor(cursorDoneColumn(), 0);
        display::print(F("OK"));
        display::setCursor(cursorCancelColumn(), 0);
        display::print(F("CA"));

        moveCursor(cursor);
    }

    bool active()
    {
        return isActive;
    }

    void loop()
    {
        if (!isActive)
            return;
        int move = input::getAndResetInputEncoder();
        if (move != 0)
        {
            if (editing)
            {
                int32_t tmp = 1;
                for (int i = 0; i < digits + fraction - cursor - 1; i++)
                {
                    tmp *= 10;
                }
                tmp = value - move * tmp;
                if (tmp < 0)
                    tmp = 0;
                value = tmp;
                printValue();
            }
            else
            {
                int newPos = cursor + move;
                if (newPos < 0)
                    newPos = 0;
                if (newPos > digits + fraction + 1)
                    newPos = digits + fraction+1;

                moveCursor(newPos);
            }
        }
        if (input::getAndResetInputEncoderClicked())
        {
            if (cursorAtDone())
            {
                isActive = false;
                success(value);
            }
            if (cursorAtCancel())
            {
                isActive = false;
                cancel();
            }
            editing = !editing;
        }
    }
}