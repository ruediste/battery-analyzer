#include "config.h"

#if IS_FRAMEWORK_ARDUINO
#include "sdLogging.h"
#include <SdFat.h>
#include <BufferedPrint.h>
#include "utils.h"
#include "batteryChannel.h"
namespace sdLogging
{
    SPIClass SPI_2 = SPIClass(PB15, PB14, PB13, PB12);
    SdFat sd;
    File logFile;
    int failure = -1;

    int getFailure()
    {
        return failure;
    }

    int getError()
    {
        return sd.sdErrorCode();
    }

    void init()
    {
        pinMode(PB12, OUTPUT);
        digitalWrite(PB12, HIGH);
    }

    instantMs_t nextPrint = 0;

    void loop()
    {
        instantMs_t now = utils::now();
        if (now > nextPrint)
        {
            nextPrint = now + 10000;

            bool printHeader = false;
            if (failure != 0)
            {
                // try to open the sd card and the file
                failure = 0;
                if (!sd.begin(SdSpiConfig(PB12, DEDICATED_SPI, SD_SCK_MHZ(2), &SPI_2)))
                {

                    failure = 1;
                    return;
                }

                if (!sd.exists("log.csv"))
                {
                    printHeader = true;
                }

                if (!logFile.open("log.csv", O_RDWR | O_CREAT | O_APPEND))
                {
                    failure = 2;
                    return;
                }
            }

            BufferedPrint<File, 64> bp(&logFile);
            if (printHeader)
            {
                bp.print(F("time"));
                for (int i = 0; i < channelCount; i++)
                {
                    bp.print(F(",Wh"));
                    bp.print(i);

                    bp.print(F(",Vm"));
                    bp.print(i);

                    bp.print(F(",Vb"));
                    bp.print(i);

                    bp.print(F(",A"));
                    bp.print(i);

                    bp.print(F(",mAh"));
                    bp.print(i);
                }
                bp.print('\n');
            }

            bp.printField(now, ',');
            for (int i = 0; i < channelCount; i++)
            {
                BatteryChannel &ch = BatteryChannel::channels[i];
                bp.printField(ch.setup().stats.wattHours(), ',', 5);
                bp.printField(ch.measuredVoltage(), ',', 3);
                bp.printField(ch.batteryVoltage(), ',', 3);
                bp.printField(ch.effectiveCurrent(), ',', 3);
                bp.printField(ch.setup().stats.milliAmperHours(), ',', 2);
            }
            bp.print('\n');

            if (!bp.sync() || !logFile.sync())
            {
                failure = 3;
            }
        }
    }
}
#endif