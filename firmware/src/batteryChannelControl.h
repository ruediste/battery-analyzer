#ifndef BATTERY_CHANNEL_CONTROL_H
#define BATTERY_CHANNEL_CONTROL_H

#include "batteryChannelHal.h"
#include "utils.h"
#include "config.h"
#include "eeprom.h"
#include "log.h"

#if IS_FRAMEWORK_NATIVE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <math.h>
#endif

/**
 * Controls a battery channel via the BatteryChannelHAL. Provides
 * the abstraction of a constant current/constant voltage source/sink.
 * */
class BatteryChannelControl
{
    instantMs_t nextUpdate = 0;
    instantMs_t nextControlUpdate = 0;

    bool lastStepWasIncrease = false;
    float stepSize = 0;

    uint16_t zeroOutputCurrentPWM()
    {
        return BatteryChannelHal::MAX_PWM / 2;
    }

    static constexpr instantMs_t updateDelayMs = 100;
    static constexpr float alpha1s = 1 - exp(-updateDelayMs / 1000.);
    static constexpr float alpha5s = 1 - exp(-updateDelayMs / 5000.);

public:
    enum class Mode
    {
        IDLE,
        SOURCE,
        SINK,
        DIRECT_PWM
    };

    enum class Target
    {
        CURRENT,
        RESISTANCE,
        POWER,
    };

    Mode mode = Mode::IDLE;
    Target target = Target::CURRENT;

    // voltage at the battery, adjusted with the current and the known contact resistance
    float batteryVoltage = 0;
    float batteryVoltage1s = 0;

    // voltage measured in the tester
    float measuredVoltage = 0;
    uint16_t measuredVoltageRaw = 0;

    float effectiveCurrent = 0;
    uint16_t outputCurrentPWM = 0;
    float effectiveCurrent1s = 0;
    float effectiveCurrent5s = 0;

    float targetCurrent = 0;    // amps
    float targetResistance = 0; // ohm
    float targetPower = 0;      // watts

    uint16_t targetDirectPWM = 0;

    float limitVoltage = 0;

    BatteryChannelHal hal;

    void init(int channel)
    {
        hal.init_inst(channel);
    }

    eeprom::ChannelConfig &config()
    {
        return eeprom::data.channelConfig[hal.channel];
    }

    void idle()
    {
        this->mode = BatteryChannelControl::Mode::IDLE;
    }

    void loop();

#if IS_FRAMEWORK_NATIVE
    void print(WINDOW *w)
    {
        wprintw(w, "control mode: ");
        switch (mode)
        {
        case BatteryChannelControl::Mode::SOURCE:
            wprintw(w, "SOURCE");
            break;
        case BatteryChannelControl::Mode::SINK:
            wprintw(w, "SINK");
            break;
        case BatteryChannelControl::Mode::IDLE:
            wprintw(w, "IDLE");
            break;
        }

        if (mode != BatteryChannelControl::Mode::IDLE)
        {
            wprintw(w, " target: ");
            switch (target)
            {
            case BatteryChannelControl::Target::CURRENT:
                wprintw(w, "CURRENT %lf", targetCurrent);
                break;
            case BatteryChannelControl::Target::RESISTANCE:
                wprintw(w, "RESISTANCE %f", targetResistance);
                break;
            case BatteryChannelControl::Target::POWER:
                wprintw(w, "POWER %f", targetPower);
                break;
            }
        }

        wprintw(w, " limitVoltage: %f stepSize: %f pwm: %i", limitVoltage, stepSize, outputCurrentPWM);
        wprintw(w, " uMeas: %f uBat: %f zeroPWM: %i", measuredVoltage, batteryVoltage, config().zeroOutputPwm(measuredVoltage));
        wprintw(w, " HAL: uInt: %f, uBat %f, capacity: %f pwm: %i current: %f\n",
                hal.voltage, hal.uBat(), hal.capacity, hal.outputPWM, hal.outputCurrent);
        wrefresh(w);
    }
#endif
};

#endif