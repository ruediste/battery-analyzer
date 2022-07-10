#ifndef MENU_H
#define MENU_H
#include "types.h"

namespace menu
{
    struct MenuHandler
    {
        // handle an item action, either print or selected
        virtual void handleMenu(uint8_t i, bool print) = 0;
         // number of items in the menu
        virtual uint8_t menuItemCount() = 0;
    };

    /**
     * enter the given menu
     * */
    void enter(MenuHandler *handler);

    /**
     * Leave the menu. The enter() and leave() calls do not have to be paired.
     * */
    void leave();

    /**
     * To be called from the loop. NOOP if not in a menu.
     * */
    void loop();

}
#endif