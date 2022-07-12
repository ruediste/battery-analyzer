#ifndef BATTERY_CHANNEL_H
#define BATTERY_CHANNEL_H

#include "config.h"
#include "batteryChannelControl.h"

/*
A battery channel controls the charge/discharge process and keeps track of the statistics (mAh, Wh etc)
*/
class BatteryChannel
{
    BatteryChannelControl control;

public:
    BatteryChannel(int channel = -1) : control(channel) {}
    void idle() { this->control.idle(); }
    void charge(float current, float voltage) { this->control.charge(current, voltage); }
    void discharge(float current, float voltage) { this->control.discharge(current, voltage); }

    float effectiveVoltage()
    {
        return control.effectiveVoltage;
    }

    float outputVoltage(){
        return control.outputVoltage;
    }
    float effectiveCurrent()
    {
        return control.effectiveCurrent;
    }

    static void loop()
    {
        for (int i = 0; i < channelCount; i++)
        {
            channels[i].control.loop();
        }
    }

    void print(){
        control.print();
    }

    static BatteryChannel channels[];
    static void init();
};

#endif