#ifndef BATTERY_CHANNEL_H
#define BATTERY_CHANNEL_H

#include "config.h"
#include "batteryChannelControl.h"

/*
A battery channel controls the charge/discharge process and keeps track of the statistics (mAh, Wh etc)
*/
class BatteryChannel
{
    instantMs_t lastStatisticsLoop = utils::now();

    eeprom::ChargeMode appliedChargeMode = eeprom::ChargeMode::Idle;
    instantMs_t lastChargeModeChange = utils::now();
    eeprom::ChannelSetup _setup;

public:
    BatteryChannelControl control;

    BatteryChannel(int channel = -1) : control(channel)
    {
        _setup.mode = eeprom::data.startupChannelMode;
    }

    eeprom::ChannelConfig &config()
    {
        return eeprom::data.channelConfig[control.hal.channel];
    }

    eeprom::ChannelSetup &setup()
    {
        // return eeprom::data.channelSetup[control.hal.channel];
        return _setup;
    }

    void idle() { this->control.idle(); }
    void charge(float current, float limit)
    {
        this->control.target = BatteryChannelControl::Target::CURRENT;
        this->control.targetCurrent = current;
        this->control.limitVoltage = limit;
        this->control.mode = BatteryChannelControl::Mode::SOURCE;
    }
    void discharge(float current, float limit)
    {
        this->control.target = BatteryChannelControl::Target::CURRENT;
        this->control.targetCurrent = -current;
        this->control.limitVoltage = limit;
        this->control.mode = BatteryChannelControl::Mode::SINK;
    }

    float batteryVoltage()
    {
        return control.batteryVoltage;
    }

    float measuredVoltage()
    {
        return control.measuredVoltage;
    }

    float effectiveCurrent()
    {
        return control.effectiveCurrent;
    }

    void loop();

    static BatteryChannel channels[];
    static void init();
};

#endif