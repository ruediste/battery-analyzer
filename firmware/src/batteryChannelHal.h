#ifndef BATTERY_CHANNEL_HAL_H
#define BATTERY_CHANNEL_HAL_H

#include "types.h"
#include "config.h"
/**
 * Abstracts away the hardware side for the batteryChannelControl
 * */
class BatteryChannelHal
{
public:
    uint8_t channel;
#if IS_FRAMEWORK_NATIVE
    uint16_t outputPWM = 0;
    float outputCurrent = 0;
    float voltage = 3.90;
    float capacity = 50; // in ampere-seconds/volt
    instantMs_t lastLoop = 0;
    float uBat()
    {
        return voltage + outputCurrent * 0.1;
    }
#endif

#if IS_STM
    static int analogInPins[];
    static int pwmPins[];
#endif
    static const uint16_t MAX_PWM = 0x0FFF;

    void init_inst(uint8_t channel)
    {
        this->channel = channel;
    }
    void setOutputPWM(uint16_t value);
    uint16_t readVoltage();
    void loop();
    static void init();
};

#endif