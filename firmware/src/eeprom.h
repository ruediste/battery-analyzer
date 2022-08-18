#ifndef EEPROM_H
#define EEPROM_H

#include "config.h"
#include "types.h"

namespace eeprom
{
    struct ChannelConfig
    {
        float adcRefVoltage=6.6;
        uint16_t zeroOutputPwm=0x7FF;

        // multiply current (A) with factor to get PWM value
        float pwmFactor=0xFFFF/4;

        // voltage to assume when sourcing current in resistance/power mode
        float resistorSourceRefVoltage=5;

        // resistance to calculate the effective shunt input voltage
        float shuntResistance=0.2;
    };

    const uint16_t MAGIC=0xE5E7;
    
    struct Data
    {
        uint16_t magic=MAGIC;

        // minimum input voltage to assume (used for max current calculation)
        float minInputVoltage = 5.f;     

        ChannelConfig channel[channelCount];
    };

    extern Data data;

#if IS_FRAMEWORK_ARDUINO
#include "eeprom.h"

#include <EEPROM.h>
    inline uint8_t read(int idx)
    {
        return EEPROM.read(idx);
    }

    inline void write(int idx, uint8_t val)
    {
        EEPROM.write(idx, val);
    }

    void init();
    void flush();
#else
    void init();
    uint8_t read(int idx);
    void write(int idx, uint8_t val);
    void flush();
#endif

}
#endif