#include "config.h"

#if IS_FRAMEWORK_NATIVE
#include "batteryChannelHal.h"
#include "utils.h"

void BatteryChannelHal::setOutputPWM(uint16_t value)
{
    outputPWM = value;
}

uint16_t BatteryChannelHal::readVoltage()
{
    return 0xFFFF * voltage / 5.;
}

void BatteryChannelHal::init()
{
}

void BatteryChannelHal::loop()
{
    instantMs_t now = utils::now();
    double elapsed = (now - lastLoop) / 1000.;
    lastLoop = now;

    const double R = 0.2;
    const double inputVoltage = 5;

    // calculate the target output current
    double outputCurrent = 4 * (outputPWM / (double)MAX_PWM - 0.5);

    // calculate the required voltage on the other side of the shunt
    double v = voltage + outputCurrent * R;

    // limit the voltage to 0..inputVoltage
    v = max(0., min(inputVoltage, v));

    // calculate the current with the limited input voltage
    outputCurrent = (v - voltage) / R;

    // update the voltage
    voltage += outputCurrent * elapsed / capacity;
}
#endif