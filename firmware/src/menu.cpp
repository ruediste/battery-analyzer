#include "menu.h"
#include "display.h"
#include "input.h"

namespace menu
{

    // index of the first item on the screen
    uint8_t scroll;

    // position on the screen (1-4)
    uint8_t pos;

    uint8_t displayedScroll;
    uint8_t displayedPos;

    MenuHandler *currentHandler = NULL;
    uint8_t currentMenuItemCount;

    void updateDisplay()
    {
        bool updatePos = displayedPos != pos;
        if (displayedScroll != scroll)
        {
            display::clear();
            for (int i = 0; i < 4; i++)
            {
                int item = i + scroll;
                display::setCursor(1, i);
                currentHandler->handleMenu(item, true);
            }
            updatePos = true;
            displayedScroll = scroll;
        }

        if (updatePos)
        {
            display::setCursor(0, displayedPos);
            display::print(" ");
            display::setCursor(0, pos);
            display::print(">");
            displayedPos = pos;
        }
    }

    void enter(MenuHandler &handler)
    {
        currentMenuItemCount = handler.menuItemCount();
        currentHandler = &handler;
        pos = 0;
        scroll = 0;
        displayedScroll = -1;
        updateDisplay();

        input::resetInputEncoderClicked();
        input::getAndResetInputEncoder();
    }

    void leave()
    {
        currentHandler = NULL;
    }

    bool active(){
        return currentHandler!=NULL;
    }
    
    void loop()
    {
        if (!active())
            return;

        int8_t tmp = input::getAndResetInputEncoder();
        if (tmp != 0)
        {
            tmp += scroll + pos;
            if (tmp < 0)
                tmp = 0;
            else if (tmp >= currentMenuItemCount)
                tmp = currentMenuItemCount - 1;
            scroll = (tmp / 4) * 4;
            pos = tmp - scroll;
            updateDisplay();
        }

        if (input::getAndResetInputEncoderClicked())
        {
            currentHandler->handleMenu(scroll + pos, false);
        }
    }
}