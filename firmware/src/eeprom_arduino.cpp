#include "config.h"

#if IS_ARDUINO
#include "eeprom.h"

namespace eeprom
{
    Data data
    Data backData;
    void init()
    {
         for (int i = 0; i < sizeof(Data); i++)
        {
            ((uint8_t *)&backData)[i] = read(i);
            ((uint8_t *)&data)[i] = read(i);
        }

        if (data.magic!=MAGIC)
        {
            data=Data();
            flush();
        }
    }

     void flush()
    {
        uint8_t *backPtr = ((uint8_t *)&backData);
        for (int i = 0; i < sizeof(Data); i++)
        {
            uint8_t value = ((uint8_t *)&data)[i] if (backPtr[i] != value)
            {
                backPtr[i] = value;
                write(i, value);
            }
        }
    }
}

#endif