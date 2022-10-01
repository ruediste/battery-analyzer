#include "controller.h"
#include "menu.h"
#include "display.h"
#include "types.h"
#include "input.h"
#include "batteryChannel.h"
#include "utils.h"
#include "numberInput.h"
#include "eeprom.h"
#include "sdLogging.h"
#include "messageDisplay.h"

namespace controller
{
    int _currentChannel = 0;

    bool showStatistics = false;

    instantMs_t lastDisplayUpdate = 0;

    BatteryChannel &currentChannel()
    {
        return BatteryChannel::channels[_currentChannel];
    }
    eeprom::ChannelSetup &currentChannelSetup()
    {
        // return eeprom::data.channelSetup[_currentChannel];
        return currentChannel().setup();
    }

    eeprom::ChannelConfig &currentChannelConfig()
    {
        return currentChannel().config();
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
        case eeprom::ChannelMode::Direct_PWM:
            display::print(F("Direct PWM"));
            break;
        }

        if (showStatistics)
        {
            display::setCursor(0, 1);
            display::print(currentChannelSetup().stats.milliAmperHours());
            display::print(F(" mAH"));

            display::setCursor(0, 2);
            display::print(currentChannelSetup().stats.wattHours());
            display::print(F(" WH"));

            display::setCursor(0, 3);
            display::print(F("Time "));
            display::print(currentChannelSetup().stats.seconds);
            display::print(F(" s"));
            return;
        }

        BatteryChannel &c = BatteryChannel::channels[_currentChannel];
        display::setCursor(0, 1);
        display::print(F("U "));
        display::print(c.batteryVoltage());
        display::print(F(" I "));
        display::print(c.effectiveCurrent());

        display::setCursor(0, 2);
        display::print(F("mAH "));
        display::print(currentChannelSetup().stats.milliAmperHours());

        if (currentChannelSetup().mode == eeprom::ChannelMode::Direct_PWM)
        {
            display::setCursor(0, 3);
            display::print(F("PWM "));
            display::print(currentChannel().control.outputCurrentPWM);
        }

        if (currentChannelSetup().mode == eeprom::ChannelMode::Charger)
        {
            display::setCursor(0, 3);
            switch (currentChannelSetup().chargeMode)
            {
            case eeprom::ChargeMode::Charge:
                display::print(F("Charging"));
                break;
            case eeprom::ChargeMode::Discharge:
                display::print(F("Discharging"));
                ;
                break;
            case eeprom::ChargeMode::Idle:
                display::print(F("Idle"));
            }
        }

        lastDisplayUpdate = utils::now();
    }

    void enterChannelMenu();
    void enterChannelConfigMenu();
    void enterGlobalConfigMenu();

    void setChannelMode(eeprom::ChannelMode mode)
    {
        currentChannelSetup().mode = mode;
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
                    display::print(F("Direct PWM"));
                else
                {
                    setChannelMode(eeprom::ChannelMode::Direct_PWM);
                }
                break;
            case 3:
                if (print)
                    display::print(F("CVCC Sink"));
                else
                {
                    setChannelMode(eeprom::ChannelMode::CV_CC_Sink);
                }
                break;
            case 4:
                if (print)
                    display::print(F("Resistor Source"));
                else
                {
                    setChannelMode(eeprom::ChannelMode::Resistor_Source);
                }
                break;
            case 5:
                if (print)
                    display::print(F("Resistor Sink"));
                else
                {
                    setChannelMode(eeprom::ChannelMode::Resistor_Sink);
                }
                break;
            case 6:
                if (print)
                    display::print(F("Power Source"));
                else
                {
                    setChannelMode(eeprom::ChannelMode::Power_Source);
                }
                break;
            case 7:
                if (print)
                    display::print(F("Power Sink"));
                else
                {
                    setChannelMode(eeprom::ChannelMode::Power_Sink);
                }
                break;

            case 8:
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
            return 9;
        }
    };

    ModeMenu modeMenu;

    struct GlobalConfigMenu : public menu::MenuHandler
    {

        void handleMenu(uint8_t i, bool print) override
        {
            switch (i)
            {
            case 0:
                if (print)
                    display::print(F("Charge Voltage"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        eeprom::data.chargeVoltage * 100, 2, 2, [](auto &c)
                        {
                        c.success = [](uint32_t value)
                        {
                            eeprom::data.chargeVoltage=value/100.;
                            eeprom::flush();
                            enterGlobalConfigMenu(); 
                        };
                        c.cancel =[]{enterGlobalConfigMenu(); }; });
                }
                break;
            case 1:
                if (print)
                    display::print(F("Charge Current"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        eeprom::data.chargeCurrent * 100, 2, 2, [](auto &c)
                        {
                        c.success = [](uint32_t value)
                        {
                            eeprom::data.chargeCurrent=value/100.;
                            eeprom::flush();
                            enterGlobalConfigMenu(); 
                        };
                        c.cancel =[]{enterGlobalConfigMenu(); }; });
                }
                break;
            case 2:
                if (print)
                    display::print(F("Ch Cutoff Current"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        eeprom::data.chargeCutoffCurrent * 100, 2, 2, [](auto &c)
                        {
                        c.success = [](uint32_t value)
                        {
                            eeprom::data.chargeCutoffCurrent=value/100.;
                            eeprom::flush();
                            enterGlobalConfigMenu(); 
                        };
                        c.cancel =[]{enterGlobalConfigMenu(); }; });
                }
                break;
            case 3:
                if (print)
                    display::print(F("Discharge Voltage"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        eeprom::data.dischargeVoltage * 100, 2, 2, [](auto &c)
                        {
                        c.success = [](uint32_t value)
                        {
                            eeprom::data.dischargeVoltage=value/100.;
                            eeprom::flush();
                            enterGlobalConfigMenu(); 
                        };
                        c.cancel =[]{enterGlobalConfigMenu(); }; });
                }
                break;
            case 4:
                if (print)
                    display::print(F("Discharge Current"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        eeprom::data.dischargeCurrent * 100, 2, 2, [](auto &c)
                        {
                        c.success = [](uint32_t value)
                        {
                            eeprom::data.dischargeCurrent=value/100.;
                            eeprom::flush();
                            enterGlobalConfigMenu(); 
                        };
                        c.cancel =[]{enterGlobalConfigMenu(); }; });
                }
                break;
            case 5:
                if (print)
                    display::print(F("Reset EEPROM"));
                else
                {
                    eeprom::data = eeprom::Data();
                    eeprom::flush();
                }
                break;
            case 6:
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
            return 7;
        }
    };

    GlobalConfigMenu globalConfigMenu;
    void enterGlobalConfigMenu()
    {
        menu::enter(globalConfigMenu);
    }

    struct ChannelConfigMenu : public menu::MenuHandler
    {
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
                        currentChannel().measuredVoltage() * 1000, 2, 3, [](auto &c)
                        {
                        c.success = [](uint32_t value)
                        {
                       currentChannelConfig().adcRefVoltage=value/1000.*0xFFFF/currentChannel().control.measuredVoltageRaw;
                       eeprom::flush();
                       enterChannelConfigMenu(); };
                        c.cancel =
                            []
                        { enterChannelConfigMenu(); };
                        c.print =
                            []
                        {
                            display::print(F("Raw ADC: "));
                            display::print(currentChannel().control.measuredVoltageRaw);
                        }; });
                }
                break;
            case 1:
                if (print)
                    display::print(F("Zero PWM High"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        currentChannelConfig().zeroOutputPwmHigh, 5, 0, [](auto &c)
                        {
                            c.success=[](uint32_t value)                           {
                                currentChannelConfig().zeroOutputPwmHigh=value;
                                currentChannelConfig().zeroOutputPwmVoltageHigh=currentChannel().measuredVoltage();
                                eeprom::flush();
                                enterChannelConfigMenu(); };
                            c.cancel=[] { enterChannelConfigMenu(); }; 
                            c.print=[]{display::print("U High: "); display::print(currentChannel().measuredVoltage());};
                            c.max=BatteryChannelHal::MAX_PWM; });
                }
                break;
            case 2:
                if (print)
                    display::print(F("Zero PWM Low"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        currentChannelConfig().zeroOutputPwmLow, 5, 0, [](auto &c)
                        {
                            c.success=[](uint32_t value)                           {
                                currentChannelConfig().zeroOutputPwmLow=value;
                                currentChannelConfig().zeroOutputPwmVoltageLow=currentChannel().measuredVoltage();
                                eeprom::flush();
                                enterChannelConfigMenu(); };
                            c.cancel=[] { enterChannelConfigMenu(); }; 
                            c.print=[]{display::print("U Low: "); display::print(currentChannel().measuredVoltage());};
                            c.max=BatteryChannelHal::MAX_PWM; });
                }
                break;
            case 3:
                if (print)
                    display::print(F("Current"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        abs(currentChannel().effectiveCurrent()) * 1000, 2, 3, [](auto &c)
                        {
                            c.success=[](uint32_t value)                               {
                            currentChannelConfig().pwmFactor=abs(currentChannel().control.outputCurrentPWM-currentChannelConfig().zeroOutputPwm(currentChannel().measuredVoltage()))/(value/1000.);
                            eeprom::flush();
                            enterChannelConfigMenu(); };
                            c.cancel=                        []                        { enterChannelConfigMenu(); };
                            c.print=                        []                        {
                                display::print(F("Output PWM: "));
                                display::print(currentChannel().control.outputCurrentPWM);
                            }; });
                }
                break;
            case 4:
                if (print)
                    display::print(F("V Bat"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        abs(currentChannel().batteryVoltage()) * 1000, 2, 3, [](auto &c)
                        {
                            c.success=[](uint32_t value)                               
                            {
                                float batteryVoltage=value/1000.;
                            currentChannelConfig().connectionResistance=(currentChannel().measuredVoltage()-batteryVoltage)/currentChannel().effectiveCurrent();
                            eeprom::flush();
                            enterChannelConfigMenu(); };
                            c.cancel= [] { enterChannelConfigMenu(); };
                            c.print= []{
                                display::print(F("U: "));
                                display::print(currentChannel().measuredVoltage());
                                display::print(F(" I: "));
                                display::print(currentChannel().effectiveCurrent());
                            }; });
                }
                break;
            case 5:
                if (print)
                    display::print(F("R Src Ref Voltage"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        currentChannelConfig().resistorSourceRefVoltage * 1000., 2, 3, [](auto &c)
                        {
                            c.success=[](uint32_t value){
                                currentChannelConfig().resistorSourceRefVoltage=value/1000.;
                                eeprom::flush();
                                enterChannelConfigMenu(); };
                            c.cancel=                                []                                { enterChannelConfigMenu(); }; });
                }
                break;
            case 6:
                if (print)
                    display::print(F("Shunt Resistance"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        currentChannelConfig().shuntResistance * 1000., 2, 3, [](auto &c)
                        { c.success = [](uint32_t value)
                          {
                       currentChannelConfig().shuntResistance=value/1000.;
                       eeprom::flush();
                       enterChannelConfigMenu(); },
                          c.cancel = []
                          { enterChannelConfigMenu(); }; });
                }
                break;
            case 7:
                if (print)
                    display::print(F("Min Input Voltage"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        eeprom::data.minInputVoltage * 1000., 2, 3, [](auto &c)
                        { c.success = [](uint32_t value)
                          {
                      eeprom::data.minInputVoltage=value/1000.;
                       eeprom::flush();
                       enterChannelConfigMenu(); };
                          c.cancel = []
                          { enterChannelConfigMenu(); }; });
                }
                break;
            case 8:
                if (print)
                    display::print(F("Global"));
                else
                {
                    menu::enter(globalConfigMenu);
                }
                break;
            case 9:
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
            return 10;
        }
    };

    ChannelConfigMenu channelConfigMenu;
    void enterChannelConfigMenu()
    {
        menu::enter(channelConfigMenu);
    }

    struct ModeMenuSuffix
    {
        // if false is returned, the caller will leave the current menu. If true is returned, a new menu or a number entry has typically been started
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
                    display::print(F("Show Statistics"));
                else
                {
                    showStatistics = true;
                }
                break;
            case 2:
                if (print)
                    display::print(F("Configure"));
                else
                {
                    menu::enter(channelConfigMenu);
                    return true;
                }
                break;
            case 3:
                if (print)
                    display::print(F("Reset Statistics"));
                else
                {
                    currentChannelSetup().stats.reset();
                }
                break;
            case 4:
                if (print)
                {
                    display::print(F("SD "));
                    display::print(sdLogging::getFailure());

                    if (sdLogging::getFailure() != 0)
                    {
                        display::print(F(" "));
                        display::print(sdLogging::getError());
                    }
                }
                else
                {
                    return true;
                }
                break;
            case 5:
                if (print)
                    display::print(F("..."));
                break;
            }

            return false;
        }

        uint8_t menuItemCount()
        {
            return 6;
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
                {
                    currentChannelSetup().chargeMode = eeprom::ChargeMode::Discharge;
                }
                break;
            case 1:
                if (print)
                    display::print(F("Charge"));
                else
                {
                    currentChannelSetup().chargeMode = eeprom::ChargeMode::Charge;
                }
                break;
            case 2:
                if (print)
                    display::print(F("Idle"));
                else
                {
                    currentChannelSetup().chargeMode = eeprom::ChargeMode::Idle;
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
                }
                break;
            case 1:
                if (print)
                    display::print(F("Current"));
                else
                {
                    menu::leave();
                    numberInput::enter(
                        currentChannelSetup().targetCurrent * 1000, 2, 3, [](auto &c)
                        { c.success = [](uint32_t value)
                          {
                              currentChannelSetup().targetCurrent = value / 1000.;
                          }; });
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
                        currentChannelSetup().limitVoltage * 1000, 2, 3, [](auto &c)
                        { c.success = [](uint32_t value)
                          {
                              currentChannelSetup().limitVoltage = value / 1000.;
                          }; });
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

    struct DirectPwmModeMenu : public menu::MenuHandler
    {
        void handleMenu(uint8_t i, bool print) override
        {
            switch (i)
            {
            case 0:
                if (print)
                {
                    display::print(F("Set PWM"));
                }
                else
                {
                    menu::leave();
                    numberInput::enter(
                        currentChannelSetup().directPWM, 5, 0, [](auto &c)
                        {
                            c.success = [](uint32_t value)
                            {
                            currentChannelSetup().directPWM = value;
                            };

                            c.valueChanged = [](uint32_t value)
                            {
                                currentChannelSetup().directPWM = value;
                               
                            };
                            c.max=BatteryChannelHal::MAX_PWM;

                            c.print = []()
                            {
                                display::print(F("I "));
                                display::print(currentChannel().effectiveCurrent());
                            }; });
                    return;
                }
                break;
            case 1:
                if (print)
                {
                    display::print(F("Set Zero PWM High"));
                }
                else
                {
                    currentChannelConfig().zeroOutputPwmHigh = currentChannelSetup().directPWM;
                    currentChannelConfig().zeroOutputPwmVoltageHigh = currentChannel().measuredVoltage();
                    eeprom::flush();
                }
                break;
            case 2:
                if (print)
                {
                    display::print(F("Set Zero PWM Low"));
                }
                else
                {
                    currentChannelConfig().zeroOutputPwmLow = currentChannelSetup().directPWM;
                    currentChannelConfig().zeroOutputPwmVoltageLow = currentChannel().measuredVoltage();
                    eeprom::flush();
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

    DirectPwmModeMenu directPwmModeMenu;

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
        case eeprom::ChannelMode::Direct_PWM:
            menu::enter(directPwmModeMenu);
            break;
        case eeprom::ChannelMode::Resistor_Source:
        case eeprom::ChannelMode::Resistor_Sink:
        case eeprom::ChannelMode::Power_Source:
        case eeprom::ChannelMode::Power_Sink:
        default:
            menu::enter(fallbackModeMenu);
            break;
        }
    }

    void init()
    {
        display::init();
        eeprom::init();
        input::init();
        BatteryChannel::init();
        sdLogging::init();
        updateDisplay();
    }

    void loop()
    {
        input::loop();
        menu::loop();
        numberInput::loop();
        sdLogging::loop();
        messageDisplay::loop();

        for (int i = 0; i < channelCount; i++)
        {
            BatteryChannel::channels[i].loop();
        }

        if (menu::active() || numberInput::active() || messageDisplay::active())
            return;

        if (utils::now() - lastDisplayUpdate > 500)
            updateDisplay();

        if (showStatistics)
        {
            if (input::getAndResetInputEncoderClicked())
            {
                input::getAndResetInputEncoder();
                showStatistics = false;
            }
            return;
        }

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