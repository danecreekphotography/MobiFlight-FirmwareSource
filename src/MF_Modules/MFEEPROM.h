//
// MFEEPROM.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <EEPROM.h>

class MFEEPROM
{
private:
    uint16_t _eepromLength = 0;

public:
    MFEEPROM();
    void     init(void);
    uint16_t get_length(void);
    uint8_t read_byte(uint16_t adr);
    bool write_byte(uint16_t adr, const uint8_t data);
    void commit() {
#if !defined(ARDUINO_ARCH_AVR)
        EEPROM.commit();
#endif
    }

    template <typename T>
    bool read_block(uint16_t adr, T &t)
    {
        if (adr + sizeof(T) > _eepromLength) return false;
        EEPROM.get(adr, t);
        return true;
    }

    template <typename T>
    bool read_block(uint16_t adr, T &t, uint16_t len)
    {
        if (adr + len > _eepromLength) return false;
        uint8_t *ptr = (uint8_t*) &t;
        for (uint16_t i = 0; i < len; i++) {
            *ptr++ = EEPROM.read(adr + i);
        }
        return true;
    }

    template <typename T>
    const bool write_block(uint16_t adr, const T &t)
    {
        if (adr + sizeof(T) > _eepromLength) return false;
        EEPROM.put(adr, t);
        return true;
    }

    template <typename T>
    const bool write_block(uint16_t adr, const T &t, uint16_t len)
    {
        if (adr + len > _eepromLength) return false;
        for (uint16_t i = 0; i < len; i++) {
            EEPROM.put(adr + i, t[i]);
        }
        return true;
    }
};

// MFEEPROM.h