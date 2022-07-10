#include "controller.h"
#include "menu.h"
#include "display.h"
#include "types.h"
#include "input.h"

namespace controller
{
    struct PositionMenu : public menu::MenuHandler
    {
        void handleMenu(uint8_t i, bool print) override
        {
            switch (i)
            {
            case 0:
                if (print)
                    display::print(F("Menu 0"));
                break;
            case 1:
                if (print)
                    display::print(F("Menu 1"));
                break;
            case 2:
                if (print)
                    display::print(F("Menu 2"));
                break;
            case 3:
                if (print)
                    display::print(F("Menu 3"));
                break;

            default:
                if (print)
                {
                    display::print(F("Menu"));
                    display::print(i, 10);
                }
                break;
            }
        }
        uint8_t menuItemCount() override
        {
            return 20;
        }
    };

    PositionMenu positionMenu;

    void init()
    {
        display::init();
        input::init();

        menu::enter(&positionMenu);
    }

    void loop()
    {
        menu::loop();
        input::loop();
    }
}