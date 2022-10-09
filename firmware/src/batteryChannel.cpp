#include "config.h"

#include "batteryChannel.h"
#include "log.h"

#if IS_FRAMEWORK_NATIVE
#define PRE_CHARGE_TIME_MS 6'000
#define PRE_DISCHARGE_TIME_MS 18'000
#else
#define PRE_CHARGE_TIME_MS 60'000
#define PRE_DISCHARGE_TIME_MS 180'000
#endif

void BatteryChannel::init()
{
    for (int channel = 0; channel < channelCount; channel++)
    {
        channels[channel] = BatteryChannel();
        channels[channel].init_inst(channel);
    }
    BatteryChannelHal::init();
}

void BatteryChannel::loop()
{
    control.loop();
    chargeController.loop();
    testController.loop();

    instantMs_t now = utils::now();

    if (now > lastStatisticsLoop + 1000)
    {
        instantMs_t elapsedMs = now - lastStatisticsLoop;
        lastStatisticsLoop = now;

        // don't run statistics while charger is idle
        if (!(setup().mode == eeprom::ChannelMode::Charger && setup().chargeMode == eeprom::ChargeMode::Idle))
        {
            float elapsedSeconds = elapsedMs / 1000.;

            setup().stats.seconds += elapsedSeconds;
            float elapsedAS = elapsedSeconds * control.effectiveCurrent1s;
            setup().stats.ampereSeconds += elapsedAS;
            setup().stats.wattSeconds += elapsedAS * control.batteryVoltage1s;
        }
    }

    switch (setup().mode)
    {
    case eeprom::ChannelMode::Charger:
    {
        if (appliedChargeMode != setup().chargeMode)
        {
            appliedChargeMode = setup().chargeMode;

            testController.done = true;
            chargeController.idle = true;

            switch (appliedChargeMode)
            {
            case eeprom::ChargeMode::Charge:
                chargeController.charge();
                break;
            case eeprom::ChargeMode::Discharge:
                chargeController.discharge();
                break;
            case eeprom::ChargeMode::Idle:
                control.mode = BatteryChannelControl::Mode::IDLE;
                break;
            case eeprom::ChargeMode::Test:
                testController.start();
                break;
            }
        }

        switch (appliedChargeMode)
        {
        case eeprom::ChargeMode::Charge:
        case eeprom::ChargeMode::Discharge:
            if (chargeController.idle)
            {
                setup().chargeMode = eeprom::ChargeMode::Idle;
            }
            break;
        case eeprom::ChargeMode::Test:
            if (testController.done)
            {
                setup().chargeMode = eeprom::ChargeMode::Idle;
            }
            break;
        }
    }
    break;
    case eeprom::ChannelMode::CV_CC_Source:
        if (setup().enabled)
        {
            control.target = BatteryChannelControl::Target::CURRENT;
            control.targetCurrent = setup().targetCurrent;
            control.limitVoltage = setup().limitVoltage;
            control.mode = BatteryChannelControl::Mode::SOURCE;
        }
        else
        {
            control.mode = BatteryChannelControl::Mode::IDLE;
        }
        break;
    case eeprom::ChannelMode::CV_CC_Sink:
        if (setup().enabled)
        {
            control.target = BatteryChannelControl::Target::CURRENT;
            control.targetCurrent = -setup().targetCurrent;
            control.limitVoltage = setup().limitVoltage;
            control.mode = BatteryChannelControl::Mode::SINK;
        }
        else
        {
            control.mode = BatteryChannelControl::Mode::IDLE;
        }
        break;
    case eeprom::ChannelMode::Resistor_Source:
        break;
    case eeprom::ChannelMode::Resistor_Sink:
        break;
    case eeprom::ChannelMode::Power_Source:
        break;
    case eeprom::ChannelMode::Power_Sink:
        break;
    case eeprom::ChannelMode::Direct_PWM:
        control.mode = BatteryChannelControl::Mode::DIRECT_PWM;
        control.targetDirectPWM = setup().directPWM;
        break;
    }
}
BatteryChannel BatteryChannel::channels[channelCount];
