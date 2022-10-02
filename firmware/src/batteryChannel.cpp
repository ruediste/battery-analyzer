#include "config.h"

#include "batteryChannel.h"

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
        channels[channel] = BatteryChannel(channel);
    }
    BatteryChannelHal::init();
}

void BatteryChannel::loop()
{
    control.loop();

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
            float elapsedAS = elapsedSeconds * control.effectiveCurrent;
            setup().stats.ampereSeconds += elapsedAS;
            setup().stats.wattSeconds += elapsedAS * control.batteryVoltage;
        }
    }

    switch (setup().mode)
    {
    case eeprom::ChannelMode::Charger:
    {
        if (appliedChargeMode != setup().chargeMode)
        {
            appliedChargeMode = setup().chargeMode;
            lastChargeModeChange = now;
            switch (appliedChargeMode)
            {
            case eeprom::ChargeMode::Charge:
                charge();
                break;
            case eeprom::ChargeMode::Discharge:
                discharge();
                break;
            case eeprom::ChargeMode::Idle:
                idle();
                break;
            case eeprom::ChargeMode::Test:
                lastTestModeChange = now;
                setup().stats.reset();
                setup().dischargeStats.reset();
                setup().testComplete = false;
                setup().testState = eeprom::TestState::PreCharge;
                charge();
                break;
            }
        }

        bool delayOk = false;
        bool charging = appliedChargeMode == eeprom::ChargeMode::Charge;
        bool discharging = appliedChargeMode == eeprom::ChargeMode::Discharge;
        switch (appliedChargeMode)
        {
        case eeprom::ChargeMode::Charge:
        case eeprom::ChargeMode::Discharge:
            delayOk = now > lastChargeModeChange + 5000;
            break;
        case eeprom::ChargeMode::Test:
            delayOk = now > lastTestModeChange + 5000;
            switch (setup().testState)
            {
            case eeprom::TestState::PreCharge:
            case eeprom::TestState::MainCharge:
                charging = true;
                break;
            case eeprom::TestState::PreDischarge:
            case eeprom::TestState::MainDischarge:
                discharging = true;
                break;
            }
            break;
        }

        if (delayOk)
        {
            // check if the charge cutoff has been reached
            if (charging)
            {
                if (abs(effectiveCurrent()) < eeprom::data.chargeCutoffCurrent)
                {
                    if (appliedChargeMode == eeprom::ChargeMode::Test)
                    {
                        switch (setup().testState)
                        {
                        case eeprom::TestState::PreCharge:
                            if (now > lastChargeModeChange + PRE_CHARGE_TIME_MS)
                                // the precharge has been running for long enough
                                setup().testState = eeprom::TestState::MainDischarge;
                            else
                                // switch to pre discharge
                                setup().testState = eeprom::TestState::PreDischarge;

                            setup().stats.reset();
                            discharge();
                            lastTestModeChange = now;
                            break;
                        case eeprom::TestState::MainCharge:
                            // done
                            setup().testComplete = true;
                            setup().chargeMode = eeprom::ChargeMode::Idle;
                            break;
                        }
                    }
                    else
                        setup().chargeMode = eeprom::ChargeMode::Idle;
                }
            }

            // check if the discharge cutoff has been reached
            if (discharging)
            {
                if (abs(effectiveCurrent()) < eeprom::data.dischargeCurrent * 0.9)
                {
                    if (appliedChargeMode == eeprom::ChargeMode::Test)
                    {
                        switch (setup().testState)
                        {
                        case eeprom::TestState::PreDischarge:
                            // the cutoff should not be reached during the pre discharge, thus switch to idle
                            setup().chargeMode = eeprom::ChargeMode::Idle;
                            break;
                        case eeprom::TestState::MainDischarge:
                            // main discharge done, switch to charging
                            setup().dischargeStats = setup().stats;
                            setup().stats.reset();
                            setup().testState = eeprom::TestState::MainCharge;
                            lastTestModeChange = now;
                            charge();
                            break;
                        }
                    }
                    else
                        setup().chargeMode = eeprom::ChargeMode::Idle;
                }
            }
        }

        if (appliedChargeMode == eeprom::ChargeMode::Test && setup().testState == eeprom::TestState::PreDischarge && now > lastTestModeChange + PRE_DISCHARGE_TIME_MS)
        {
            // pre discharge done, switch back to pre-charge
            setup().testState = eeprom::TestState::PreCharge;
            setup().stats.reset();
            lastTestModeChange = now;
            charge();
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
