#ifndef BATTERY_CHANNEL_CONTROL_H
#define BATTERY_CHANNEL_CONTROL_H

#include "batteryChannelHal.h"
#include "utils.h"
#include "config.h"

#if IS_FRAMEWORK_NATIVE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#endif

/**
 * Controls a battery channel via the BatteryChannelHAL. Provides
 * the abstraction of a constant current/constant voltage source/sink.
 * */
class BatteryChannelControl
{
    float inputVoltage = 5.f;             // TODO: read
    float shuntResistance = 0.2f;         // TODO: configure
    float resistorSourceRefVoltage = 5.f; // TODO: configure

    float adcRefVoltage = 5; // volt
    float maxCurrent = 2;    // ampere

    instantMs_t nextUpdate = 0;

    bool lastStepWasIncrease = false;
    float stepSize = 0;

    uint16_t outputCurrentPWM = 0;

    uint16_t zeroOutputCurrentPWM(){
        return BatteryChannelHal::MAX_PWM/2;
    }

public:
    enum class Mode
    {
        IDLE,
        SOURCE,
        SINK,
    };

    enum class Target
    {
        CURRENT,
        RESISTANCE,
        POWER,
    };

    Mode mode = Mode::IDLE;
    Target target = Target::CURRENT;

    float effectiveVoltage = 0;
    float effectiveCurrent = 0;

    float targetCurrent = 0;    // amps
    float targetResistance = 0; // ohm
    float targetPower = 0;      // watts

    float limitVoltage = 0;

    BatteryChannelHal hal;
    BatteryChannelControl(int channel) : hal(BatteryChannelHal(channel))
    {
    }

    void idle()
    {
        this->mode = BatteryChannelControl::Mode::IDLE;
    }

    void loop();

    void print(WINDOW *w)
    {
#if IS_FRAMEWORK_NATIVE
        wprintw(w, "mode: ");
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
                wprintw(w, "CURRENT %f limit: %f", targetCurrent);
                break;
            case BatteryChannelControl::Target::RESISTANCE:
                wprintw(w, "RESISTANCE %f", targetResistance);
                break;
            case BatteryChannelControl::Target::POWER:
                wprintw(w, "POWER %f", targetPower);
                break;
            }
        }

        wprintw(w, " limitVoltage: %f outputCurrent: %i stepSize: %f",
                limitVoltage, outputCurrentPWM,stepSize);
        wprintw(w, " HAL: voltage: %f capacity: %f pwm: %i\n",
                hal.voltage, hal.capacity, hal.outputPWM);
        wrefresh(w);
#endif
    }
};

#endif