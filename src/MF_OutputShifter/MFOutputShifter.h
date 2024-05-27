//
// MFOutputShifter.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>

#ifdef REVERSED_OUTPUT_OUTPUTSHIFTER
    #define MF_HIGH LOW
    #define MF_LOW  HIGH
#else
    #define MF_HIGH HIGH
    #define MF_LOW  LOW
#endif

class MFOutputShifter
{
public:
    MFOutputShifter();
    void setPin(uint8_t pin, uint8_t value, uint8_t refresh = 1);
    void setPins(char *pins, uint8_t value);
    bool attach(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t moduleCount);
    void detach();
    void clear();
    void update();

private:
    uint8_t _latchPin;    // Latch pin
    uint8_t _clockPin;    // Clock pin
    uint8_t _dataPin;     // Data/SI pin
    uint8_t _moduleCount; // Number of 8 bit modules in series. For a shift register with 16 bit one needs to select 2 modules a 8......
    uint8_t *_lastState;
    bool    _initialized = false;
};

// MFOutputShifter.h
