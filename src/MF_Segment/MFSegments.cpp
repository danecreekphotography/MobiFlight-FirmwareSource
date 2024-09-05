//
// MFSegments.cpp
//
// (C) MobiFlight Project 2022
//

#include "MFSegments.h"
#include "commandmessenger.h"

MFSegments::MFSegments()
{
    _moduleCount = 0;
}

void MFSegments::display(uint8_t module, char *string, uint8_t points, uint8_t mask, bool convertPoints)
{
    if (_moduleCount == 0)
        return;
    uint8_t digit = 8;
    uint8_t pos   = 0;
    for (uint8_t i = 0; i < 8; i++) {
        digit--;
        if (((1 << digit) & mask) == 0)
            continue;
        _ledControl.setChar(module, digit, string[pos], ((1 << digit) & points));
        pos++;
    }
}

void MFSegments::setSingleSegment(uint8_t module, uint8_t segment, uint8_t on_off)
{
    if (_moduleCount == 0)
        return;
    
    _ledControl.setSingleSegment(module, segment, on_off);

}

void MFSegments::setBrightness(uint8_t module, uint8_t value)
{
    if (_moduleCount == 0)
        return;

    if (module < _moduleCount) {
        if (value) {
            _ledControl.setIntensity(module, value - 1);
            _ledControl.shutdown(module, false);
        } else {
            _ledControl.shutdown(module, true);
        }
    }
}

bool MFSegments::attach(uint8_t type, uint8_t dataPin, uint8_t csPin, uint8_t clkPin, uint8_t moduleCount, uint8_t brightness)
{
    if (!_ledControl.begin(type, dataPin, clkPin, csPin, moduleCount))
        return false;

    _moduleCount = moduleCount;

    for (uint8_t i = 0; i < _moduleCount; ++i) {
        setBrightness(i, brightness);
        _ledControl.shutdown(i, false);
        _ledControl.clearDisplay(i);
    }

    return true;
}

void MFSegments::detach()
{
    _moduleCount = 0;
}

void MFSegments::powerSavingMode(bool state)
{
    for (uint8_t i = 0; i != _moduleCount; ++i) {
        _ledControl.shutdown(i, state);
    }
}

void MFSegments::test()
{
    if (_moduleCount == 0)
        return;
    uint8_t _delay = 10;
    uint8_t module = 0;
    uint8_t digit  = 0;

    for (digit = 0; digit < 8; digit++) {
        for (module = 0; module != _moduleCount; ++module) {
            _ledControl.setDigit(module, digit, 8, 1);
        }
        delay(_delay);
    }

    for (digit = 0; digit < 8; digit++) {
        for (module = 0; module != _moduleCount; ++module) {
            _ledControl.setChar(module, 7 - digit, '-', false);
        }
        delay(_delay);
    }

    for (digit = 0; digit < 8; digit++) {
        for (module = 0; module != _moduleCount; ++module) {
            _ledControl.setChar(module, 7 - digit, ' ', false);
        }
        delay(_delay);
    }
}

// MFSegments.cpp
