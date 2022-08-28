#include "controller.h"
#include "menu.h"
#include "display.h"
#include "types.h"
#include "input.h"
#include "batteryChannel.h"
#include "utils.h"
#include "numberInput.h"
#include "eeprom.h"

namespace controller
{
    int _currentChannel = 0;

    instantMs_t lastDisplayUpdate = 0;

    eeprom::ChannelSetup &currentChannelSetup()
    {
        return eeprom::data.channelSetup[_currentChannel];
    }

    void updateDisplay()
    {
        display::clear();
        display::setCursor(0, 0);
        display::print(F("CH: "));
        display::print(_currentChannel);
        display::print(F(" M: "));

        switch (currentChannelSetup().mode)
        {
        case eeprom::ChannelMode::Charger:
            display::print(F("Charger"));
            break;
        case eeprom::ChannelMode::CV_CC_Source:
            display::print(F("CV CC Src"));
            break;
        case eeprom::ChannelMode::CV_CC_Sink:
            display::print(F("CV CC Sink"));
            break;
        case eeprom::ChannelMode::Resistor_Source:
            display::print(F("Res Source"));
            break;
        case eeprom::ChannelMode::Resistor_Sink:
            display::print(F("Res Sink"));
            break;
        case eeprom::ChannelMode::Power_Source:
            display::print(F("Power Source"));
            break;
        case eeprom::ChannelMode::Power_Sink:
            display::print(F("Power Sink"));
            break;
        }

        BatteryChannel &c = BatteryChannel::channels[_currentChannel];
        display::setCursor(0, 1);
        display::print(F("U "));
        display::print(c.effectiveVoltage());
        display::print(F(" I "));
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

    void updateCurrentChannel()
    {
        switch (currentChannelSetup().mode)
        {
        case eeprom::ChannelMode::Charger:
            break;
        case eeprom::ChannelMode::CV_CC_Source:
            if (currentChannelSetup().enabled)
            {
                currentChannel().control.target = BatteryChannelControl::Target::CURRENT;
                currentChannel().control.targetCurrent = currentChannelSetup().targetCurrent;
                currentChannel().control.limitVoltage = currentChannelSetup().limitVoltage;
                currentChannel().control.mode = BatteryChannelControl::Mode::SOURCE;
            }
            else
            {
                currentChannel().control.mode = BatteryChannelControl::Mode::IDLE;
            }
            break;
        case eeprom::ChannelMode::CV_CC_Sink:
            if (currentChannelSetup().enabled)
            {
                currentChannel().control.target = BatteryChannelControl::Target::CURRENT;
                currentChannel().control.targetCurrent = -currentChannelSetup().targetCurrent;
                currentChannel().control.limitVoltage = currentChannelSetup().limitVoltage;
                currentChannel().control.mode = BatteryChannelControl::Mode::SINK;
            }
            else
            {
                currentChannel().control.mode = BatteryChannelControl::Mode::IDLE;
            }
            break;
        case eeprom::ChannelMode::Resistor_Source:
            break;
        case eeprom::ChannelMode::Resistor_Sink:
            break;
        case eeprom::ChannelMode::Power_Source:
            break;
        case eeprom::ChannelMode::Power_Sink:
            break;
        }
    }

    void setChannelMode(eeprom::ChannelMode mode)
    {
        currentChannelSetup().mode = mode;
        eeprom::flush();
        updateCurrentChannel();
        menu::leave();
    }

    struct ModeMenu : public menu::MenuHandler
    {

        void handleMenu(uint8_t i, bool print) override
        {
            switch (i)
            {
            case 0:
                if (print)
                    display::print(F("Charger"));
                else
                {
                    setChannelMode(eeprom::ChannelMode::Charger);
                }
                break;
            case 1:
                if (print)
                    display::print(F("CVCC Source"));
                else
                {
                    setChannelMode(eeprom::ChannelMode::CV_CC_Source);
                }
                break;
            case 2:
                if (print)
                    display::print(F("CVCC Sink"));
                else
                {
                    setChannelMode(eeprom::ChannelMode::CV_CC_Sink);
                }
                break;
            case 3:
                if (print)
                    display::print(F("Resistor Source"));
                else
                {
                    setChannelMode(eeprom::ChannelMode::Resistor_Source);
                }
                break;
            case 4:
                if (print)
                    display::print(F("Resistor Sink"));
                else
                {
                    setChannelMode(eeprom::ChannelMode::Resistor_Sink);
                }
                break;
            case 5:
                if (print)
                    display::print(F("Power Source"));
                else
                {
                    setChannelMode(eeprom::ChannelMode::Power_Source);
                }
                break;
            case 6:
                if (print)
                    display::print(F("Power Sink"));
                else
                {
                    setChannelMode(eeprom::ChannelMode::Power_Sink);
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

        uint8_t menuItemCount() override
        {
            return 8;
        }
    };

    ModeMenu modeMenu;

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
                       currentChannelConfig().pwmFactor=abs(currentChannel().control.outputCurrentPWM-currentChannelConfig().zeroOutputPwm)/(value/1000.);
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
                    menu::enter(globalConfigMenu);
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
        menu::enter(channelConfigMenu);
    }

    struct ModeMenuSuffix
    {
        bool handleMenu(uint8_t i, uint8_t count, bool print)
        {
            switch (i + menuItemCount() - count)
            {
            case 0:
                if (print)
                    display::print(F("Mode"));
                else
                {
                    menu::enter(modeMenu);
                    return true;
                }
                break;
            case 1:
                if (print)
                    display::print(F("Configure"));
                else
                {
                    menu::enter(channelConfigMenu);
                    return true;
                }
                break;
            case 2:
                if (print)
                    display::print(F("..."));
                break;
            }

            return false;
        }

        uint8_t menuItemCount()
        {
            return 3;
        }
    };

    ModeMenuSuffix modeMenuSuffix;

    struct ChargeModeMenu : public menu::MenuHandler
    {
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
            default:
                if (modeMenuSuffix.handleMenu(i, menuItemCount(), print))
                    return;
            }

            if (!print)
            {
                // leave menu after most clicks
                menu::leave();
                updateDisplay();
            }
        }

        uint8_t menuItemCount() override
        {
            return 3 + modeMenuSuffix.menuItemCount();
        }
    };

    ChargeModeMenu chargeModeMenu;

    struct CvCCModeMenu : public menu::MenuHandler
    {
        void handleMenu(uint8_t i, bool print) override
        {
            switch (i)
            {

            case 0:
                if (print)
                {
                    if (currentChannelSetup().enabled)
                        display::print(F("OFF"));
                    else
                        display::print(F("ON"));
                }
                else
                {
                    currentChannelSetup().enabled = !currentChannelSetup().enabled;
                    eeprom::flush();
                    updateCurrentChannel();
                }
                break;
            case 1:
                if (print)
                    display::print(F("Current"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        currentChannelSetup().targetCurrent * 1000, 2, 3, [](uint32_t value)
                        {
                            currentChannelSetup().targetCurrent=value/1000.;
                            eeprom::flush(); 
                            updateCurrentChannel(); },
                        [] {},
                        [] {});
                    return;
                }
                break;
            case 2:
                if (print)
                    display::print(F("Voltage"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        currentChannelSetup().limitVoltage * 1000, 2, 3, [](uint32_t value)
                        {
                            currentChannelSetup().limitVoltage=value/1000.;
                            eeprom::flush(); 
                            updateCurrentChannel(); },
                        [] {},
                        [] {});
                    return;
                }
                break;
            default:
                if (modeMenuSuffix.handleMenu(i, menuItemCount(), print))
                    return;
            }

            if (!print)
            {
                // leave menu after most clicks
                menu::leave();
                updateDisplay();
            }
        }

        uint8_t menuItemCount() override
        {
            return 3 + modeMenuSuffix.menuItemCount();
        }
    };

    CvCCModeMenu cvCCModeMenu;

    struct FallbackModeMenu : public menu::MenuHandler
    {
        void handleMenu(uint8_t i, bool print) override
        {
            switch (i)
            {

            case 0:
                if (print)
                {
                    display::print(F("Fallback"));
                }
                break;

            default:
                if (modeMenuSuffix.handleMenu(i, menuItemCount(), print))
                    return;
            }

            if (!print)
            {
                // leave menu after most clicks
                menu::leave();
                updateDisplay();
            }
        }

        uint8_t menuItemCount() override
        {
            return 1 + modeMenuSuffix.menuItemCount();
        }
    };

    FallbackModeMenu fallbackModeMenu;

    void enterChannelMenu()
    {
        switch (currentChannelSetup().mode)
        {
        case eeprom::ChannelMode::Charger:
            menu::enter(chargeModeMenu);
            break;
        case eeprom::ChannelMode::CV_CC_Source:
        case eeprom::ChannelMode::CV_CC_Sink:
            menu::enter(cvCCModeMenu);
            break;
        case eeprom::ChannelMode::Resistor_Source:
        case eeprom::ChannelMode::Resistor_Sink:
        case eeprom::ChannelMode::Power_Source:
        case eeprom::ChannelMode::Power_Sink:
            menu::enter(fallbackModeMenu);
            break;
        }
    }

    void init()
    {
        eeprom::init();
        display::init();
        input::init();
        BatteryChannel::init();
        updateDisplay();
        for (int i = 0; i < channelCount; i++)
        {
            _currentChannel = i;
            if (currentChannelSetup().mode > eeprom::ChannelMode::Power_Sink)
            {
                currentChannelSetup().mode = eeprom::ChannelMode::Charger;
            }
            updateCurrentChannel();
        }
        _currentChannel = 0;
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

        int move = input::getAndResetInputEncoder();
        if (move != 0)
        {
            _currentChannel += move;
            if (_currentChannel < 0)
                _currentChannel = 0;
            if (_currentChannel >= channelCount)
                _currentChannel = channelCount - 1;
            updateDisplay();
        }
        if (input::getAndResetInputEncoderClicked())
        {
            enterChannelMenu();
        }
    }
}