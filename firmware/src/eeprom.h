#ifndef EEPROM_H
#define EEPROM_H

#include "config.h"
#include "types.h"

#if IS_FRAMEWORK_ARDUINO
#include <EEPROM.h>
#endif

namespace eeprom
{
    struct ChannelConfig
    {
        float adcRefVoltage = 6.6;
        uint16_t zeroOutputPwm = 0x7FF;

        // multiply current (A) with factor to get PWM value
        float pwmFactor = 0x0FFF / 4;

        // voltage to assume when sourcing current in resistance/power mode
        float resistorSourceRefVoltage = 5;

        // resistance to calculate the effective shunt input voltage
        float shuntResistance = 0.2;
    };

    enum class ChannelMode
    {
        Charger,
        CV_CC_Source,
        CV_CC_Sink,
        Resistor_Source,
        Resistor_Sink,
        Power_Source,
        Power_Sink,
    };

    enum class ChargeMode
    {
        Charge,
        Discharge,
        Idle
    };

    struct ChannelSetup
    {
        ChannelMode mode;

        ChargeMode chargeMode;

        // not used for charger mode
        bool enabled;

        float targetCurrent = 0; // amps
        float limitVoltage = 0;

        float targetResistance = 0; // ohm
        float targetPower = 0;      // watts
    };

    const uint16_t MAGIC = 0xE5E7;
    const uint16_t VERSION = 0;

    struct Data
    {
        uint16_t magic = MAGIC;
        uint16_t version = VERSION;

        // minimum input voltage to assume (used for max current calculation)
        float minInputVoltage = 5.f;

        ChannelConfig channelConfig[channelCount];
        ChannelSetup channelSetup[channelCount];
    };

    extern Data data;

#if IS_FRAMEWORK_ARDUINO

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