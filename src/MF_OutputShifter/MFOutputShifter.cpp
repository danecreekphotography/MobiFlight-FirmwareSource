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
        _lastState[idx] |= msk;
    } else {
        _lastState[idx] &= ~msk;
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
    _initialized  = true;
#ifdef USE_FAST_IO
    _latchPin.Port = portOutputRegister(digitalPinToPort(latchPin));
    _latchPin.Mask = digitalPinToBitMask(latchPin);
    _clockPin.Port = portOutputRegister(digitalPinToPort(clockPin));
    _clockPin.Mask = digitalPinToBitMask(clockPin);
    _dataPin.Port  = portInputRegister(digitalPinToPort(dataPin));
    _dataPin.Mask  = digitalPinToBitMask(dataPin);
#else
    _latchPin    = latchPin;
    _clockPin    = clockPin;
    _dataPin     = dataPin;
#endif
    _moduleCount  = moduleCount;

    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);

    if (!FitInMemory(sizeof(uint8_t) * _moduleCount))
        return false;

    _lastState = new (allocateMemory(sizeof(uint8_t) * _moduleCount)) uint8_t;

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
        _lastState[i] = 0xFF * MF_LOW;
    }
    update();
}

void MFOutputShifter::update()
{
    DIGITALWRITE(_latchPin, LOW);
    for (uint8_t i = _moduleCount; i > 0; i--) {
        for (int8_t j = 7; j >= 0; j--) {
            DIGITALWRITE(_dataPin, (_lastState[i - 1] & (1 << (j))));
            DIGITALWRITE(_clockPin, HIGH);
            DIGITALWRITE(_clockPin, LOW);
        }
    }
    DIGITALWRITE(_latchPin, HIGH);
}

void MFOutputShifter::powerSavingMode(bool state)
{
    if (state) {
        DIGITALWRITE(_latchPin, LOW);
        for (uint8_t i = _moduleCount; i > 0; i--) {
            for (int8_t j = 7; j >= 0; j--) {
                DIGITALWRITE(_dataPin, MF_LOW);
                DIGITALWRITE(_clockPin, HIGH);
                DIGITALWRITE(_clockPin, LOW);
            }
        }
        DIGITALWRITE(_latchPin, HIGH);
    } else {
        update();
    }
}

// MFOutputShifter.cpp
