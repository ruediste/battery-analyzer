#include "config.h"

#include "batteryChannel.h"

void BatteryChannel::init()
{
    for (int channel = 0; channel < channelCount; channel++)
    {
        channels[channel] = BatteryChannel(channel);
    }
    BatteryChannelHal::init();
}

void BatteryChannel::loop()
{
    control.loop();

    instantMs_t now = utils::now();

    if (now + 1000 > lastStatisticsLoop)
    {
        instantMs_t elapsedMs = now - lastStatisticsLoop;
        lastStatisticsLoop = now;

        float elapsedSeconds = elapsedMs / 1000.;

        setup().stats.seconds += elapsedSeconds;
        float elapsedAS = elapsedSeconds * control.effectiveCurrent;
        setup().stats.ampereSeconds += elapsedAS;
        setup().stats.wattSeconds += elapsedAS * control.batteryVoltage;
    }

    if (now + 10000 > lastEepromFlush)
    {
        lastEepromFlush = now;
        eeprom::flush();
    }

    switch (setup().mode)
    {
    case eeprom::ChannelMode::Charger:
        if (appliedChargeMode != setup().chargeMode)
        {
            appliedChargeMode = setup().chargeMode;
            lastChargeModeChange = now;
            switch (appliedChargeMode)
            {
            case eeprom::ChargeMode::Charge:
                charge(eeprom::data.chargeCurrent, eeprom::data.chargeVoltage);
                break;
            case eeprom::ChargeMode::Discharge:
                discharge(eeprom::data.dischargeCurrent, eeprom::data.dischargeVoltage);
                break;
            case eeprom::ChargeMode::Idle:
                idle();
            }
        }
        // check if the charge cutoff has been reached
        if (lastChargeModeChange + 5000 < now)
        {
            if (appliedChargeMode == eeprom::ChargeMode::Charge)
            {
                if (abs(effectiveCurrent()) < eeprom::data.chargeCutoffCurrent)
                {
                    setup().chargeMode = eeprom::ChargeMode::Idle;
                }
            }
            if (appliedChargeMode == eeprom::ChargeMode::Discharge)
            {
                if (abs(effectiveCurrent()) < eeprom::data.dischargeCurrent * 0.9)
                {
                    setup().chargeMode = eeprom::ChargeMode::Idle;
                }
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
