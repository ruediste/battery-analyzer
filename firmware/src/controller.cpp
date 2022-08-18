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
    int _currentChannel = 0;

    instantMs_t lastDisplayUpdate = 0;

    void updateDisplay()
    {
        display::clear();
        display::setCursor(0, 0);
        display::print(F("Channel "));
        display::print(_currentChannel);

        BatteryChannel &c = BatteryChannel::channels[_currentChannel];
        display::setCursor(0, 1);
        display::print(F("VBat "));
        display::print(c.effectiveVoltage());

        display::setCursor(0, 2);
        display::print(F("Current "));
        display::print(c.effectiveCurrent());

        lastDisplayUpdate = utils::now();
    }

    void enterChannelMenu();
    void enterChannelConfigMenu();

    BatteryChannel &currentChannel()
    {
        return BatteryChannel::channels[_currentChannel];
    }

    eeprom::ChannelConfig &currentChannelConfig()
    {
        return currentChannel().config();
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
                    eeprom::data = eeprom::Data();
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
            return 8;
        }

        void handleMenu(uint8_t i, bool print) override
        {
            switch (i)
            {
            case 0:
                if (print)
                    display::print(F("Input Voltage"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        currentChannel().effectiveVoltage() * 1000, 2, 3, [](uint32_t value)
                        {
                       currentChannelConfig().adcRefVoltage=value/1000.*0xFFFF/currentChannel().control.effectiveVoltageRaw;
                       eeprom::flush();
                       enterChannelConfigMenu(); },
                        []
                        { enterChannelConfigMenu(); },
                        []
                        {
                            display::print(F("Raw ADC: "));
                            display::print(currentChannel().control.effectiveVoltageRaw);
                        });
                }
                break;
            case 1:
                if (print)
                    display::print(F("Zero Output PWM"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        currentChannelConfig().zeroOutputPwm, 5, 0, [](uint32_t value)
                        {
                       currentChannelConfig().zeroOutputPwm=value;
                       eeprom::flush();
                       enterChannelConfigMenu(); },
                        []
                        { enterChannelConfigMenu(); },
                        [] {});
                }
                break;
            case 2:
                if (print)
                    display::print(F("Current"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        abs(currentChannel().effectiveCurrent()) * 1000, 2, 3, [](uint32_t value)
                        {
                       currentChannelConfig().pwmFactor=(currentChannel().control.outputCurrentPWM-currentChannelConfig().zeroOutputPwm)/(value/1000.);
                       eeprom::flush();
                       enterChannelConfigMenu(); },
                        []
                        { enterChannelConfigMenu(); },
                        []
                        {
                            display::print(F("Output PWM: "));
                            display::print(currentChannel().control.outputCurrentPWM);
                        });
                }
                break;
            case 3:
                if (print)
                    display::print(F("Resistor Source Ref Voltage"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        currentChannelConfig().resistorSourceRefVoltage * 1000., 2, 3, [](uint32_t value)
                        {
                       currentChannelConfig().resistorSourceRefVoltage=value/1000.;
                       eeprom::flush();
                       enterChannelConfigMenu(); },
                        []
                        { enterChannelConfigMenu(); },
                        [] {});
                }
                break;
            case 4:
                if (print)
                    display::print(F("Shunt Resistance"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        currentChannelConfig().shuntResistance * 1000., 2, 3, [](uint32_t value)
                        {
                       currentChannelConfig().shuntResistance=value/1000.;
                       eeprom::flush();
                       enterChannelConfigMenu(); },
                        []
                        { enterChannelConfigMenu(); },
                        [] {});
                }
                break;
            case 5:
                if (print)
                    display::print(F("Min Input Voltage"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        eeprom::data.minInputVoltage * 1000., 2, 3, [](uint32_t value)
                        {
                      eeprom::data.minInputVoltage=value/1000.;
                       eeprom::flush();
                       enterChannelConfigMenu(); },
                        []
                        { enterChannelConfigMenu(); },
                        [] {});
                }
                break;
            case 6:
                if (print)
                    display::print(F("Global"));
                else
                {
                    menu::enter(&globalConfigMenu);
                }
                break;
            case 7:
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
    void enterChannelConfigMenu()
    {
        menu::enter(&channelConfigMenu);
    }

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
                    currentChannel().discharge(1, 3);
                break;
            case 1:
                if (print)
                    display::print(F("Charge"));
                else
                    currentChannel().charge(1, 4);
                break;
            case 2:
                if (print)
                    display::print(F("Idle"));
                else
                    currentChannel().idle();
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

        int move=input::getAndResetInputEncoder();
        if (move!=0){
            _currentChannel+=move;
            if (_currentChannel<0)
            _currentChannel=0;
            if (_currentChannel>=channelCount)
            _currentChannel=channelCount-1;
            updateDisplay();
        }
        if (input::getAndResetInputEncoderClicked())
        {
            menu::enter(&channelMenu);
        }
    }
}