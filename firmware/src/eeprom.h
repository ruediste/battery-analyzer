#ifndef EEPROM_H
#define EEPROM_H

#include "config.h"
#include "types.h"
#include "batteryChannelHal.h"

#if IS_FRAMEWORK_ARDUINO
#include <EEPROM.h>
#endif

namespace eeprom
{
    struct ChannelConfig
    {
        float adcRefVoltage = 6.6;

        // multiply current (A) with factor to get PWM value
        float pwmFactor = 0x0FFF / 4;

        // voltage to assume when sourcing current in resistance/power mode
        float resistorSourceRefVoltage = 5;

        // resistance to calculate the effective shunt input voltage
        float shuntResistance = 0.2;

        // resistance between the voltage measurement point and the battery (plus and minus side combined)
        float connectionResistance = 0.1;

        float zeroOutputPwmVoltageHigh = 4.2;
        float zeroOutputPwmVoltageLow = 2.8;
        uint16_t zeroOutputPwmHigh = 0x7FF;
        uint16_t zeroOutputPwmLow = 0x7FF;

        uint16_t zeroOutputPwm(float voltage)
        {
            return zeroOutputPwmLow + (voltage - zeroOutputPwmVoltageLow) * (zeroOutputPwmHigh - zeroOutputPwmLow) / (zeroOutputPwmVoltageHigh - zeroOutputPwmVoltageLow);
        }
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
        Test,
        Charge,
        Discharge,
        Idle,
    };

    struct Statistics
    {
        float ampereSeconds = 0;

        float milliAmperHours()
        {
            return ampereSeconds * 1000 / 3600;
        }
        float wattSeconds = 0;

        float wattHours()
        {
            return wattSeconds / 3600;
        }

        float seconds = 0;

        void reset()
        {
            ampereSeconds = 0;
            wattSeconds = 0;
            seconds = 0;
        }
    };

    struct ChannelSetup
    {
        ChannelMode mode = ChannelMode::Direct_PWM;

        ChargeMode chargeMode = ChargeMode::Idle;

        // not used for charger mode
        bool enabled = false;

        float targetCurrent = 0; // amps
        float limitVoltage = 0;

        float targetResistance = 0; // ohm
        float targetPower = 0;      // watts

        uint16_t directPWM = BatteryChannelHal::MAX_PWM / 2;

        Statistics stats;
        Statistics dischargeStats;

        float efficiencyPercent()
        {
            return (dischargeStats.wattSeconds / dischargeStats.ampereSeconds) / (stats.wattSeconds / stats.ampereSeconds) * 100;
        }

        float capacityDifferencePercent()
        {
            return 100 * (-dischargeStats.ampereSeconds / stats.ampereSeconds - 1);
        }
    };

    const uint16_t MAGIC = 0xE5E7;
    const uint16_t VERSION = 0;

    struct Data
    {
        uint16_t magic = MAGIC;
        uint16_t version = VERSION;

        // minimum input voltage to assume (used for max current calculation)
        float minInputVoltage = 5.f;

        float chargeVoltage = 4.15;
        float chargeCurrent = 1;
        float chargeCutoffCurrent = 0.2;
        float dischargeVoltage = 3.3;
        float dischargeCurrent = 1;

        ChannelConfig channelConfig[channelCount];

        eeprom::ChannelMode startupChannelMode;
        float dischargeCutoffCurrent = 0.2;
    };

    extern Data data;

    void init();
    void flush();
}
#endif