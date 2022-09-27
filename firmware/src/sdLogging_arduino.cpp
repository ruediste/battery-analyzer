#include "config.h"

#if IS_FRAMEWORK_ARDUINO
#include "sdLogging.h"
#include <SdFat.h>
#include <BufferedPrint.h>
#include "utils.h"
namespace sdLogging
{
    SPIClass SPI_2 = SPIClass(PB15, PB14, PB13, PB12);
    SdFat sd;
    File logFile;
    int failure = 0;

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

        failure = 0;
        // SPI_2.begin();

        if (!sd.begin(SdSpiConfig(PB12, DEDICATED_SPI, SD_SCK_MHZ(2), &SPI_2)))
        {

            failure = 1;
            return;
        }

        if (!logFile.open("log.txt", O_RDWR | O_CREAT | O_APPEND))
        {
            //   Serial.print("open csvFile failed");
            failure = 2;
            return;
        }

        BufferedPrint<File, 64> bp(&logFile);
        bp.print("Time");
        if (!bp.sync() || !logFile.sync())
        {
            // Serial.print("sync csvFile failed");
            failure = 3;
            return;
        }
    }

    instantMs_t nextPrint = 0;

    void loop()
    {
        if (failure != 0)
            return;
        instantMs_t now = utils::now();
        if (now > nextPrint)
        {
            nextPrint = now + 1000;

            BufferedPrint<File, 64> bp(&logFile);
            bp.printField(now, '\n');
            if (!bp.sync() || !logFile.sync())
            {
                //      Serial.print("sync csvFile failed");
                failure = 4;
            }
        }
    }
}
#endif