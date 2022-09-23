#include "config.h"

#if IS_FRAMEWORK_ARDUINO
#include "eeprom.h"

#define FLASH_BANK_NUMBER FLASH_BANK_1
#define FLASH_END FLASH_BANK1_END
#define FLASH_BASE_ADDRESS ((uint32_t)((FLASH_END + 1) - FLASH_PAGE_SIZE))

namespace eeprom
{
    Data data;
    void init()
    {
        data = Data();
        return;

        memcpy(&data, (uint8_t *)(FLASH_BASE_ADDRESS), sizeof(Data));

        if (data.magic != MAGIC || data.version != VERSION)
        {
            data = Data();
            flush();
        }
    }

    void flush()
    {
        FLASH_EraseInitTypeDef EraseInitStruct;
        uint32_t offset = 0;
        uint32_t address = FLASH_BASE_ADDRESS;
        uint32_t address_end = FLASH_BASE_ADDRESS + sizeof(Data);
        uint32_t pageError = 0;

        EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.Banks = FLASH_BANK_NUMBER;
        EraseInitStruct.PageAddress = FLASH_BASE_ADDRESS;
        EraseInitStruct.NbPages = 1;

        if (HAL_FLASH_Unlock() == HAL_OK)
        {
            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
            if (HAL_FLASHEx_Erase(&EraseInitStruct, &pageError) == HAL_OK)
            {
                while (address <= address_end)
                {

                    uint64_t tmp = *((uint64_t *)((uint8_t *)&data + offset));

                    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, tmp) == HAL_OK)
                    {
                        address += 8;
                        offset += 8;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            HAL_FLASH_Lock();
        }
    }

    static_assert(sizeof(Data) < FLASH_PAGE_SIZE, "Size is not correct");
}

#endif