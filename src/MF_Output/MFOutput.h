//
// MFOutput.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>

#if defined(REVERSED_OUTPUT_OUTPUT)
#define MF_HIGH LOW
#define MF_LOW  HIGH
#else
#define MF_HIGH HIGH
#define MF_LOW  LOW
#endif
class MFOutput
{
public:
    MFOutput();
    void attach(uint8_t pin);
    void set(uint8_t value);
    void powerSavingMode(bool state);

private:
    uint8_t _pin;
    uint8_t _value;
};

// MFOutput.h
