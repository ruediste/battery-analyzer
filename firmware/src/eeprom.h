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

        // resistance between the voltage measurement point and the battery (plus and minus side combined)
        float connectionResistance = 0.1;
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
        Direct_PWM,
    };

    enum class ChargeMode
    {
        Charge,
        Discharge,
        Idle
    };

    enum class TestState
    {
        // At the start of the test cycle, the battery has to charge for at least 30 seconds before the charge current drops to the
        // charge cutoff current. If charging completes before that, the PreDischarge state is entered. Otherwise the discharge starts
        PreCharge,

        // The battery voltage is too high. The battery is discharged (typically for 2 Minutes), before another PreCharge attempt is made
        PreDischarge,

        // The battery is being discharged. In the middle of the discharge, the discharge voltage delta (dischargeDU) is measured
        Discharge,

        // The battery is being charged. In the middle of the discharge, the charge voltage delta (chargeDU) is measured
        Charge,
    };

    struct Statistics
    {
        float ampereSeconds;

        float milliAmperHours()
        {
            return ampereSeconds * 1000 / 3600;
        }
        float wattSeconds;

        float wattHours()
        {
            return wattSeconds / 3600;
        }

        float seconds;

        void reset()
        {
            ampereSeconds = 0;
            wattSeconds = 0;
            seconds = 0;
        }
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

        uint16_t directPWM = 0;

        Statistics stats;

        Statistics dischargeStats;

        float dischargeDU;
        float chargeDU;
    };

    const uint16_t MAGIC = 0xE5E7;
    const uint16_t VERSION = 0;

    struct Data
    {
        uint16_t magic = MAGIC;
        uint16_t version = VERSION;

        // minimum input voltage to assume (used for max current calculation)
        float minInputVoltage = 5.f;

        float chargeVoltage = 4;
        float chargeCurrent = 1;
        float chargeCutoffCurrent = 0.5;
        float dischargeVoltage = 3.2;
        float dischargeCurrent = 1;

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