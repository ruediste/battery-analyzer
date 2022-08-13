#include "controller.h"
#include "menu.h"
#include "display.h"
#include "types.h"
#include "input.h"
#include "batteryChannel.h"
#include "utils.h"

namespace controller
{
    int currentChannel = 0;

    instantMs_t lastDisplayUpdate = 0;

    void updateDisplay()
    {
        display::clear();
        display::setCursor(0, 0);
        display::print(F("Channel "));
        display::print(currentChannel);

        BatteryChannel &c = BatteryChannel::channels[currentChannel];
        display::setCursor(0, 1);
        display::print(F("VBat "));
        display::print(c.effectiveVoltage());

        display::setCursor(0, 2);
        display::print(F("Current "));
        display::print(c.effectiveCurrent());

        lastDisplayUpdate = utils::now();
    }

    struct PositionMenu : public menu::MenuHandler
    {
        uint8_t menuItemCount() override
        {
            return 4;
        }

        void handleMenu(uint8_t i, bool print) override
        {
            switch (i)
            {
            case 0:
                if (print)
                    display::print(F("..."));
                break;
            case 1:
                if (print)
                    display::print(F("Discharge"));
                else
                    BatteryChannel::channels[currentChannel].discharge(1, 3);
                break;
            case 2:
                if (print)
                    display::print(F("Charge"));
                else
                    BatteryChannel::channels[currentChannel].charge(1, 4);
                break;
            case 3:
                if (print)
                    display::print(F("Idle"));
                else
                    BatteryChannel::channels[currentChannel].idle();
                break;

            default:
                break;
            }

            // exit menu after a click
            if (!print)
            {
                menu::leave();
                updateDisplay();
            }
        }
    };

    PositionMenu positionMenu;

    void init()
    {
        display::init();
        input::init();
        BatteryChannel::init();
        updateDisplay();
    }

    void loop()
    {
        menu::loop();
        input::loop();
        BatteryChannel::loop();
        if (menu::active())
            return;

        if (utils::now() - lastDisplayUpdate > 500)
            updateDisplay();

        if (input::getAndResetInputEncoderClicked())
        {
            menu::enter(&positionMenu);
        }
    }
}