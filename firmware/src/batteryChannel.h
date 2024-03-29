#ifndef BATTERY_CHANNEL_H
#define BATTERY_CHANNEL_H

#include "config.h"
#include "batteryChannelControl.h"
#include "messageDisplay.h"
#include "log.h"

#if IS_FRAMEWORK_NATIVE
#define PRE_CHARGE_TIME_MS 6'000
#define PRE_DISCHARGE_TIME_MS 18'000
#define SETTLE_TIME_MS 4'000
#define REDUCE_LIMIT_MAX_MS 10000
#else
#define PRE_CHARGE_TIME_MS 60'000
#define PRE_DISCHARGE_TIME_MS 180'000
#define SETTLE_TIME_MS 20'000
#endif

/**
 * Cutoff 0.1A:
 * 0:
 * Discharge: -1826mAh, -6.248Wh 225 min
 * Charge:  ?, 7.2 Wh, 172 min
 * Total: 400 min, 6.5h
 *
 * 1:  751mAh
 *
 * Cutoff 0.2A:
 * 0:
 * Discharge: -1414 mAh, -4.8 Wh, 167 min
 * Charge: 1345.36 mAh, 5.4 Wh, 117 min
 * eff: 87.63; diff: 5.1%
 *
 * 1:
 * Discharge: -1115 mAh, -3.85 Wh, 109 min
 * Charge: 1037mAh, 4.28 Wh, 103 min
 * eff: 90%; diff: 7.5%
 *
 * Cutoff 0.2A + Pulse
 *
 * 0:
 * Discharge: -1351, -4.51 Wh, 154 min
 *
 * 1:
 * Discharge: -1103 mAh, -3.76 Wh, 100 min
 * Charge 939 mAh 3.89 Wh 72 min
 * 96.75%
 * */

/**
 * Controls the charging/discharge process of a
 * */
class ChargeController
{
    float limit;
    instantMs_t nextDecision;
    bool charging;

    BatteryChannelControl *control;

public:
    bool idle = true;
    void init(BatteryChannelControl *c)
    {
        this->control = c;
    }

    void charge()
    {
        limit = eeprom::data.chargeVoltage;
        charging = true;
        idle = false;

        control->mode = BatteryChannelControl::Mode::SOURCE;
        control->target = BatteryChannelControl::Target::CURRENT;
        control->targetCurrent = eeprom::data.chargeCurrent;
        control->limitVoltage = limit;

        nextDecision = utils::now() + SETTLE_TIME_MS;
    }

    void discharge()
    {
        limit = eeprom::data.dischargeVoltage;
        charging = false;
        idle = false;

        control->mode = BatteryChannelControl::Mode::SINK;
        control->target = BatteryChannelControl::Target::CURRENT;
        control->targetCurrent = -eeprom::data.dischargeCurrent;
        control->limitVoltage = limit;

        nextDecision = utils::now() + SETTLE_TIME_MS;
    }

    void loop()
    {
        if (idle)
            return;
        instantMs_t now = utils::now();

        if (now > nextDecision)
        {
            bool currentBelowCutoff = (charging && control->effectiveCurrent5s < eeprom::data.chargeCutoffCurrent) || (!charging && -control->effectiveCurrent5s < eeprom::data.dischargeCutoffCurrent);

            if (currentBelowCutoff)
            {
                LOG("ChargeController current5s: %f loop(): switch to idle", control->effectiveCurrent5s);
                idle = true;
            }
            else
            {
                // check every second without limit extension
                nextDecision = now + 1000;
            }
        }
    }
};

/**
 * Controls the test process of a battery
 * */

class TestController
{
public:
    enum class State
    {
        // At the start of the test cycle, the battery has to charge for at least 30 seconds before the charge current drops to the
        // charge cutoff current. If charging completes before that, the PreDischarge state is entered. Otherwise the discharge starts
        PreCharge,

        // The battery voltage is too high. The battery is discharged (typically for 2 Minutes), before another PreCharge attempt is made
        PreDischarge,

        // The battery is being discharged. In the middle of the discharge, the discharge voltage delta (dischargeDU) is measured
        MainDischarge,

        // The battery is being charged. In the middle of the discharge, the charge voltage delta (chargeDU) is measured
        MainCharge,
    };

    State state = State::PreCharge;
    ChargeController *ctrl;
    eeprom::ChannelSetup *setup;
    instantMs_t nextDecision;

    bool dischargeStatsPresent = false;
    bool completeStatsPresent = false;
    bool done = true;

    void init(ChargeController *ctrl, eeprom::ChannelSetup *setup)
    {
        this->ctrl = ctrl;
        this->setup = setup;
    }

    void start()
    {
        state = State::PreCharge;
        ctrl->charge();
        nextDecision = utils::now() + PRE_CHARGE_TIME_MS;
        setup->stats.reset();
        dischargeStatsPresent = false;
        completeStatsPresent = false;
        done = false;
    }

    void loop()
    {
        if (done)
        {
            return;
        }
        instantMs_t now = utils::now();

        if (now < nextDecision)
        {
            if (ctrl->idle)
            {
                if (state == State::PreCharge)
                {
                    // we went idle during pre charge, switch to pre discharge
                    state = State::PreDischarge;
                    setup->stats.reset();
                    ctrl->discharge();
                    nextDecision = now + PRE_DISCHARGE_TIME_MS;
                }
                else if (state == State::PreDischarge)
                {
                    // we went idle during pre discharge, something is wrong
                    done = true;
                }
            }
        }
        else
        {
            if (state == State::PreDischarge)
            {
                // the pre discharge time is up, switch back to pre-charge
                state = State::PreCharge;
                setup->stats.reset();
                ctrl->charge();
                nextDecision = utils::now() + PRE_CHARGE_TIME_MS;
            }
            else if (ctrl->idle)
            {
                switch (state)
                {
                case State::PreCharge:
                    state = State::MainDischarge;
                    setup->stats.reset();
                    ctrl->discharge();
                    break;
                case State::MainCharge:
                    done = true;
                    completeStatsPresent = true;
                    break;
                case State::MainDischarge:
                    setup->dischargeStats = setup->stats;
                    dischargeStatsPresent = true;
                    setup->stats.reset();
                    state = State::MainCharge;
                    ctrl->charge();
                    break;
                }
                nextDecision = now + SETTLE_TIME_MS;
            }
        }
    }
};

/*
A battery channel controls the charge/discharge process and keeps track of the statistics (mAh, Wh etc)
*/
class BatteryChannel
{
    instantMs_t lastStatisticsLoop = utils::now();

    eeprom::ChargeMode appliedChargeMode = eeprom::ChargeMode::Idle;

    eeprom::ChannelSetup _setup;

    ChargeController chargeController;
    TestController testController;

public:
    BatteryChannelControl control;

    void init_inst(int channel)
    {
        control.init(channel);
        chargeController.init(&control);
        testController.init(&chargeController, &_setup);
        _setup.mode = eeprom::data.startupChannelMode;
    }

    eeprom::ChannelConfig &config()
    {
        return eeprom::data.channelConfig[control.hal.channel];
    }

    eeprom::ChannelSetup &setup()
    {
        // return eeprom::data.channelSetup[control.hal.channel];
        return _setup;
    }

    TestController::State testState()
    {
        return testController.state;
    }

    bool dischargeStatsPresent() { return testController.dischargeStatsPresent; }
    bool completeStatsPresent() { return testController.completeStatsPresent; }

    void idle() { this->control.idle(); }
    void resetStatistics()
    {
        testController.dischargeStatsPresent = false;
        testController.completeStatsPresent = false;
    }

    float batteryVoltage()
    {
        return control.batteryVoltage;
    }

    float measuredVoltage()
    {
        return control.measuredVoltage;
    }

    float effectiveCurrent()
    {
        return control.effectiveCurrent;
    }

    float effectiveCurrent1s()
    {
        return control.effectiveCurrent1s;
    }

    float effectiveCurrent5s()
    {
        return control.effectiveCurrent5s;
    }

    void loop();

    static BatteryChannel channels[];
    static void init();
};

#endif