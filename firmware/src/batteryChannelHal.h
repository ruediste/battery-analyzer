#ifndef BATTERY_CHANNEL_HAL_H
#define BATTERY_CHANNEL_HAL_H

#include "types.h"
#include "config.h"
/**
 * Abstracts away the hardware side for the batteryChannelControl
 * */
class BatteryChannelHal
{
    uint8_t channel;
public:
#if IS_FRAMEWORK_NATIVE
    uint16_t outputPWM=0;
    bool dischargeEnabled=false;
    float voltage=0;
    float capacity=10; // in ampere-seconds/volt
    instantMs_t lastLoop=0;
#endif
    BatteryChannelHal(uint8_t channel)
    {
        this->channel = channel;
    }
    void setDischargeEnabled(bool value);
    void setOutputPWM(uint16_t value);
    uint16_t readVoltage();
    void loop();
    static void init();
};

#endif