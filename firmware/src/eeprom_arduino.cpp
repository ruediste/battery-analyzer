#include "config.h"

#if IS_FRAMEWORK_ARDUINO
#include "eeprom.h"

namespace eeprom
{
    Data data;
    Data backData;
    void init()
    {
        data = Data();
        return;

        for (int i = 0; i < sizeof(Data); i++)
        {
            ((uint8_t *)&backData)[i] = read(i);
            ((uint8_t *)&data)[i] = read(i);
        }

        if (data.magic != MAGIC || data.version != VERSION)
        {
            data = Data();
            flush();
        }
    }

    void flush()
    {
        return;

        uint8_t *backPtr = ((uint8_t *)&backData);
        for (int i = 0; i < sizeof(Data); i++)
        {
            uint8_t value = ((uint8_t *)&data)[i];
            if (backPtr[i] != value)
            {
                backPtr[i] = value;
                write(i, value);
            }
        }
    }

    static_assert(sizeof(Data) < 800, "Size is not correct");
}

#endif