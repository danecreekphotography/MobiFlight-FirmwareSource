//
// MFOutputShifter.cpp
//
// (C) MobiFlight Project 2022
//

#include "MFOutputShifter.h"
#include "allocateMem.h"

MFOutputShifter::MFOutputShifter()
{
    _initialized = false;
}

void MFOutputShifter::setPin(uint8_t pin, uint8_t value, uint8_t refresh)
{
    if (!_initialized) return;

    uint8_t idx = (pin & 0xF8) >> 3;
    uint8_t msk = (0x01 << (pin & 0x07));

    if (value != MF_LOW) {
        _outputBuffer[idx] |= msk;
    } else {
        _outputBuffer[idx] &= ~msk;
    }
    if (refresh) update();
}

void MFOutputShifter::setPins(char *pins, uint8_t value)
{
    if (!_initialized) return;

    char *pinTokens = strtok(pins, "|");
    while (pinTokens != 0) {
        uint8_t num = (uint8_t)atoi(pinTokens);
        setPin(num, value, 0);
        pinTokens = strtok(0, "|");
    }
    update();
}

bool MFOutputShifter::attach(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t moduleCount)
{
    _initialized = true;
    _latchPin    = latchPin;
    _clockPin    = clockPin;
    _dataPin     = dataPin;
    _moduleCount = moduleCount;

    pinMode(_latchPin, OUTPUT);
    pinMode(_clockPin, OUTPUT);
    pinMode(_dataPin, OUTPUT);

    if (!FitInMemory(sizeof(uint8_t) * _moduleCount))
        return false;

    _outputBuffer = new (allocateMemory(sizeof(uint8_t) * _moduleCount)) uint8_t;

    clear();

    return true;
}

void MFOutputShifter::detach()
{
    _initialized = false;
}

void MFOutputShifter::clear()
{
    for (uint8_t i = 0; i < _moduleCount; i++) {
        _outputBuffer[i] = 0xFF * MF_LOW;
    }
    update();
}

void MFOutputShifter::update()
{
    digitalWrite(_latchPin, LOW);
    for (uint8_t i = _moduleCount; i > 0; i--) {
        shiftOut(_dataPin, _clockPin, MSBFIRST, _outputBuffer[i - 1]); // LSBFIRST, MSBFIRST,
    }
    digitalWrite(_latchPin, HIGH);
}

// MFOutputShifter.cpp
