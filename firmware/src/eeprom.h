#ifndef EEPROM_H
#define EEPROM_H

#include "config.h"
#include "types.h"

namespace eeprom
{
    struct ChannelConfig
    {
        float adcRefVoltage=5;
    };

    const uint16_t MAGIC=0xE5E7;
    
    struct Data
    {
        uint16_t magic=MAGIC;
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