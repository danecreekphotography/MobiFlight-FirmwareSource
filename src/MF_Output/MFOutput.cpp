//
// Output.cpp
//
// (C) MobiFlight Project 2022
//

#include "MFOutput.h"

MFOutput::MFOutput()
{
    _value = false;
}

void MFOutput::attach(uint8_t pin)
{
    _pin = pin;
#ifdef ARDUINO_ARCH_RP2040
    pinMode(_pin, OUTPUT_12MA);
#else
    pinMode(_pin, OUTPUT);
#endif
    set(LOW);
}

void MFOutput::set(uint8_t value)
{
    _value = value;
    if (_value == 0xFF)
        digitalWrite(_pin, MF_HIGH);
    else if (_value == 0x00)
        digitalWrite(_pin, MF_LOW);
    else
        analogWrite(_pin, _value);
}

void MFOutput::powerSavingMode(bool state)
{
    if (state)
        set(0);
    else
        set(_value);
}

// Output.cpp
