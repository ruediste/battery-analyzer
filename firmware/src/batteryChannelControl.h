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

const float refVoltage = 5;
const float shuntResistance = 0.2;

/**
 * Controls a battery channel via the BatteryChannelHAL. Provides
 * the abstraction of a constant current/constant voltage source/sink.
 * */
class BatteryChannelControl
{
    float targetCurrent = 0;
    float limitVoltage = 0;

    instantMs_t nextUpdate = 0;

    void setOutputVoltage(float value)
    {
        if (value != outputVoltage)
        {
            outputVoltage = value;
            hal.setOutputPWM(0xFFFF * max(0.f, min(value / refVoltage, 1.f)));
        }
    }

    bool dischargeEnabled = false;
    void setDischargeEnabled(bool value)
    {
        if (value != dischargeEnabled)
        {
            dischargeEnabled = value;
            hal.setDischargeEnabled(value);
        }
    }

public:
    enum class Mode
    {
        IDLE,
        CHARGE,
        DISCHARGE
    };
    Mode mode = Mode::IDLE;
    float effectiveVoltage = 0;
    float effectiveCurrent = 0;
    float outputVoltage = 0;

    BatteryChannelHal hal;
    BatteryChannelControl(int channel) : hal(BatteryChannelHal(channel))
    {
    }

    void idle()
    {
        this->mode = BatteryChannelControl::Mode::IDLE;
    }
    void charge(float current, float voltage)
    {
        mode = BatteryChannelControl::Mode::CHARGE;
        targetCurrent = current;
        limitVoltage = voltage;
    }
    void discharge(float current, float voltage)
    {
        mode = BatteryChannelControl::Mode::DISCHARGE;
        targetCurrent = current;
        limitVoltage = voltage;
    }

    void loop();

    void print(WINDOW *w)
    {
#if IS_FRAMEWORK_NATIVE
        wprintw(w, "mode: ");
        switch (mode)
        {
        case BatteryChannelControl::Mode::CHARGE:
            wprintw(w, "CHARGE");
            break;
        case BatteryChannelControl::Mode::DISCHARGE:
            wprintw(w, "DISCHARGE");
            break;
        case BatteryChannelControl::Mode::IDLE:
            wprintw(w, "IDLE");
            break;
        }
        wprintw(w, " targetCurrent: %f limitVoltage: %f outputVoltage: %f discharge: %i\n",
                targetCurrent, limitVoltage, outputVoltage, dischargeEnabled);
        wprintw(w, "voltage: %f capacity: %f pwm: %i discharge: %i\n",
                hal.voltage, hal.capacity, hal.outputPWM, hal.dischargeEnabled);
        wrefresh(w);
#endif
    }
};

#endif