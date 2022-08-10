#include "config.h"

#if IS_FRAMEWORK_NATIVE
#include "eeprom.h"
#include <fcntl.h>
#include <sys/mman.h>

namespace eeprom
{
    uint8_t *buffer;
    void init()
    {
        int eepromFile = open("eeprom.dat", O_CREAT | O_RDWR,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

        posix_fallocate(eepromFile, 0, 4096);

        buffer = reinterpret_cast<uint8_t *>(
            mmap(NULL, 4096, PROT_WRITE, MAP_SHARED, eepromFile, 0));
    }

    uint8_t read(int idx)
    {
        return buffer[idx];
    }

    void write(int idx, uint8_t val)
    {
        buffer[idx]=val;
        
    }
}

#endif