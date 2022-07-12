#include "config.h"

#if IS_FRAMEWORK_NATIVE
#include "batteryChannelHal.h"
#include "utils.h"

void BatteryChannelHal::setDischargeEnabled(bool value)
{
    dischargeEnabled = value;
}
void BatteryChannelHal::setOutputPWM(uint16_t value)
{
    outputPWM = value;
}
uint16_t BatteryChannelHal::readVoltage()
{
    return 0xFFFF*voltage/5.;
}

void BatteryChannelHal::init()
{}
void BatteryChannelHal::loop()
{
    instantMs_t now=utils::now();
    double elapsed=(now-lastLoop)/1000.;
    lastLoop=now;

    double outputVoltage=outputPWM*5./0xFFFF;
    if (outputVoltage>voltage || dischargeEnabled){
        voltage+=(outputVoltage-voltage)/0.2*elapsed;
    }
}
#endif