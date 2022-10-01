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
        measuredVoltageRaw = hal.readVoltage();
        measuredVoltage = measuredVoltageRaw * config().adcRefVoltage / 0xFFFF;

        // calculate the effective current flowing
        {
            float roundedOutputCurrent = ((int32_t)outputCurrentPWM - config().zeroOutputPwm(measuredVoltage)) / config().pwmFactor;

            // calculate the required voltage on the other side of the shunt
            float shuntInput = measuredVoltage + roundedOutputCurrent * config().shuntResistance;

            // limit the voltage to 0..inputVoltage
            shuntInput = max(0.f, min(eeprom::data.minInputVoltage, shuntInput));

            // calculate the current with the limited input voltage
            effectiveCurrent = (shuntInput - measuredVoltage) / config().shuntResistance;
        }

        // calculate the battery voltage
        batteryVoltage = measuredVoltage - effectiveCurrent * config().connectionResistance;
    }

    if (now > nextControlUpdate)
    {
        if (abs(batteryVoltage - limitVoltage) < 0.1)
            nextControlUpdate = now + 200; // reduce control loop speed when close to the limit
        else
            nextControlUpdate = now + 100;

        // adjst the target current based on the mode
        bool increaseCurrent = false;
        switch (mode)
        {
        case BatteryChannelControl::Mode::IDLE:
            stepSize = 1;
            outputCurrentPWM = zeroOutputCurrentPWM();
            hal.setOutputPWM(outputCurrentPWM);
            return; // exit method, we're done

        case BatteryChannelControl::Mode::DIRECT_PWM:
            stepSize = 1;
            outputCurrentPWM = targetDirectPWM;
            hal.setOutputPWM(outputCurrentPWM);
            return; // exit method, we're done

        case Mode::SOURCE:
        {
            if (effectiveCurrent < 0)
                increaseCurrent = true;
            else if (batteryVoltage > limitVoltage)
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
                    i = (config().resistorSourceRefVoltage - batteryVoltage) / targetResistance;
                }
                break;
                case Target::POWER:
                    i = targetPower / batteryVoltage;
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
            else if (batteryVoltage < limitVoltage)
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
                    i = (0 - batteryVoltage) / targetResistance;
                }
                break;
                case Target::POWER:
                    i = -targetPower / (config().resistorSourceRefVoltage - batteryVoltage);
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
            int32_t tmp = outputCurrentPWM + stepSize;
            if (tmp >= BatteryChannelHal::MAX_PWM)
                outputCurrentPWM = BatteryChannelHal::MAX_PWM;
            else
                outputCurrentPWM = tmp;
        }
        else
        {
            if (lastStepWasIncrease)
                stepSize /= 2;
            else
                stepSize *= 1.41;
            stepSize = max(stepSize, 1.f); // by one step minimum
            int32_t tmp = outputCurrentPWM - stepSize;
            if (tmp < 0)
                outputCurrentPWM = 0;
            else
                outputCurrentPWM = tmp;
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
