#include "batteryChannelControl.h"
#include "utils.h"
#include "display.h"

void BatteryChannelControl::loop()
{
    hal.loop();
    instantMs_t now = utils::now();
    if (now > nextUpdate)
    {
        nextUpdate = now + 100;

        // read back the voltage
        effectiveVoltage = hal.readVoltage() * config().adcRefVoltage / 0xFFFF;

        // calculate the effective current flowing
        {
            float roundedOutputCurrent = 2 * maxCurrent * (outputCurrentPWM / (float)BatteryChannelHal::MAX_PWM - 0.5);

            // calculate the required voltage on the other side of the shunt
            float shuntInput = effectiveVoltage + roundedOutputCurrent * shuntResistance;

            // limit the voltage to 0..inputVoltage
            shuntInput = max(0.f, min(inputVoltage, shuntInput));

            // calculate the current with the limited input voltage
            effectiveCurrent = (shuntInput - effectiveVoltage) / shuntResistance;
        }

        // adjst the target current based on the mode
        bool increaseCurrent = false;
        switch (mode)
        {
        case BatteryChannelControl::Mode::IDLE:
            stepSize = 1;
            outputCurrentPWM = zeroOutputCurrentPWM();
            hal.setOutputPWM(outputCurrentPWM);
            return; // exit method, we're done

        case Mode::SOURCE:
        {
            if (effectiveCurrent < 0)
                increaseCurrent = true;
            else if (effectiveVoltage > limitVoltage)
                increaseCurrent = false;
            else
            {
                float i;
                switch (target)
                {
                case Target::CURRENT:
                {
                    i = targetCurrent;
                }
                break;

                case Target::RESISTANCE:
                {
                    i = (resistorSourceRefVoltage - effectiveVoltage) / targetResistance;
                }
                break;
                case Target::POWER:
                    i = targetPower / (resistorSourceRefVoltage - effectiveVoltage);
                    break;
                }
                if (effectiveCurrent < i)
                    increaseCurrent = true;
                else
                    increaseCurrent = false;
            }
        }
        break;
        case Mode::SINK:
        {
            if (effectiveCurrent > 0)
                increaseCurrent = false;
            else if (effectiveVoltage < limitVoltage)
                increaseCurrent = true;
            else
            {
                float i;
                switch (target)
                {
                case Target::CURRENT:
                {
                    i = targetCurrent;
                }
                break;

                case Target::RESISTANCE:
                {
                    i = (0 - effectiveVoltage) / targetResistance;
                }
                break;
                case Target::POWER:
                    i = targetPower / (resistorSourceRefVoltage - effectiveVoltage);
                    break;
                }
                if (effectiveCurrent < i)
                    increaseCurrent = true;
                else
                    increaseCurrent = false;
            }
        }
        break;
        }

        // adjust output current using exponential search
        if (increaseCurrent)
        {
            if (lastStepWasIncrease)
                stepSize *= 1.41;
            else
                stepSize /= 2;
            stepSize = max(stepSize, 1.f); // by one step minimum
            outputCurrentPWM += stepSize;
        }
        else
        {
            if (lastStepWasIncrease)
                stepSize /= 2;
            else
                stepSize *= 1.41;
            stepSize = max(stepSize, 1.f); // by one step minimum
            outputCurrentPWM -= stepSize;  // TODO: handle overflow
        }
        lastStepWasIncrease = increaseCurrent;

        // limit step size
        if (stepSize > BatteryChannelHal::MAX_PWM / 32)
            stepSize = BatteryChannelHal::MAX_PWM / 32;

        // make sure no current is sunk in source mode and vice versa
        uint16_t zeroPwm = zeroOutputCurrentPWM();
        if ((mode == Mode::SOURCE && outputCurrentPWM < zeroPwm) || (mode == Mode::SINK && outputCurrentPWM > zeroPwm))
        {
            outputCurrentPWM = zeroPwm;
            stepSize = 1;
        }
        hal.setOutputPWM(outputCurrentPWM);
    }
}
