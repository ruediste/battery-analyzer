#ifndef EEPROM_H
#define EEPROM_H

#include "config.h"
#include "types.h"

namespace eeprom {
#if IS_FRAMEWORK_ARDUINO
#include "eeprom.h"

#include <EEPROM.h>

inline uint8_t read(int idx){
    return EEPROM.read(idx);
}

inline void write(int idx, uint8_t val){
    EEPROM.write(idx, val);
}
inline void init();
#else
    void init();
    uint8_t read(int idx);
    void write(int idx, uint8_t val);
#endif

}
#endif