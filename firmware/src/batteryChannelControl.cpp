#include "batteryChannelControl.h"
#include "utils.h"

void BatteryChannelControl::loop()
{
    hal.loop();
    instantMs_t now = utils::now();
    if (now > nextUpdate)
    {
        effectiveVoltage = hal.readVoltage() * refVoltage / 0xFFFF;
        if (outputVoltage > effectiveVoltage || dischargeEnabled)
            effectiveCurrent = (outputVoltage - effectiveVoltage) / shuntResistance;
        else
            effectiveCurrent = 0;
        // printf("effectiveVoltage: %f, effectiveCurrent: %f\n",effectiveVoltage,effectiveCurrent);
        nextUpdate = now + 100;

        switch (mode)
        {
        case BatteryChannelControl::Mode::CHARGE:
        {
            setDischargeEnabled(false);
            float dc = targetCurrent - effectiveCurrent;
            float v = outputVoltage + (dc * shuntResistance);
            v = min(limitVoltage, v);
            // printf("dc: %f, v: %f\n",dc,v);
            setOutputVoltage(v);
        }
        break;
        case BatteryChannelControl::Mode::DISCHARGE:
        {
            effectiveCurrent *= -1;
            setDischargeEnabled(true);
            float dc = targetCurrent - effectiveCurrent;
            float v = outputVoltage - (dc * shuntResistance);
            v = max(limitVoltage, v);
            setOutputVoltage(v);
        }
        break;
        case BatteryChannelControl::Mode::IDLE:
            setDischargeEnabled(false);
            setOutputVoltage(0);
            break;
        }
    }
}
