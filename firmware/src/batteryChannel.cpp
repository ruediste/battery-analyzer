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

BatteryChannel BatteryChannel::channels[channelCount];
