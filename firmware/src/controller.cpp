#include "controller.h"
#include "menu.h"
#include "display.h"
#include "types.h"
#include "input.h"
#include "batteryChannel.h"
#include "utils.h"
#include "numberInput.h"

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

    void enterChannelMenu();

    eeprom::ChannelConfig &currentChannelConfig(){
        return BatteryChannel::channels[currentChannel].config();
    }

    struct GlobalConfigMenu : public menu::MenuHandler
    {
        uint8_t menuItemCount() override
        {
            return 2;
        }

        void handleMenu(uint8_t i, bool print) override
        {
            switch (i)
            {
            case 0:
                if (print)
                    display::print(F("Reset EEPROM"));
                else
                {
                    eeprom::data=eeprom::Data();
                    eeprom::flush();
                }
                break;
            case 1:
                if (print)
                    display::print(F("..."));
                else
                {
                    enterChannelMenu();
                }
                break;
            }
        }
    };

    GlobalConfigMenu globalConfigMenu;

    struct ChannelConfigMenu : public menu::MenuHandler
    {
        uint8_t menuItemCount() override
        {
            return 3;
        }

        void handleMenu(uint8_t i, bool print) override
        {
            switch (i)
            {
            case 0:
                if (print)
                    display::print(F("Calibrate Voltage"));
                else
                {
                    menu::leave();
                    numberInput::enter(currentChannelConfig().adcRefVoltage*1000,2,3,[](uint32_t value){
                       currentChannelConfig().adcRefVoltage=value/1000.;
                       eeprom::flush();
                    }, []{});
                }
                    ; 
                break;
            case 1:
                if (print)
                    display::print(F("Global"));
                else
                {
                    menu::enter(&globalConfigMenu);
                }
                break;
            case 2:
                if (print)
                    display::print(F("..."));
                else
                {
                    enterChannelMenu();
                }
                break;
            }
        }
    };

    ChannelConfigMenu channelConfigMenu;

    struct ChannelMenu : public menu::MenuHandler
    {
        uint8_t menuItemCount() override
        {
            return 5;
        }

        void handleMenu(uint8_t i, bool print) override
        {
            switch (i)
            {

            case 0:
                if (print)
                    display::print(F("Discharge"));
                else
                    BatteryChannel::channels[currentChannel].discharge(1, 3);
                break;
            case 1:
                if (print)
                    display::print(F("Charge"));
                else
                    BatteryChannel::channels[currentChannel].charge(1, 4);
                break;
            case 2:
                if (print)
                    display::print(F("Idle"));
                else
                    BatteryChannel::channels[currentChannel].idle();
                break;
            case 3:
                if (print)
                    display::print(F("Configure"));
                else
                {
                    menu::enter(&channelConfigMenu);
                    return;
                }
                break;
            case 4:
                if (print)
                    display::print(F("..."));
                break;
            }

            if (!print)
            {
                // leave menu after most clicks
                menu::leave();
                updateDisplay();
            }
        }
    };

    ChannelMenu channelMenu;

    void enterChannelMenu()
    {
        menu::enter(&channelMenu);
    }

    void init()
    {
        eeprom::init();
        display::init();
        input::init();
        BatteryChannel::init();
        updateDisplay();
    }

    void loop()
    {
        input::loop();
        menu::loop();
        numberInput::loop();
        BatteryChannel::loop();
        if (menu::active() || numberInput::active())
            return;

        if (utils::now() - lastDisplayUpdate > 500)
            updateDisplay();

        if (input::getAndResetInputEncoderClicked())
        {
            menu::enter(&channelMenu);
        }
    }
}