//
// MFServo.h
//
// (C) MobiFlight Project 2022
//

#include "MFServo.h"

void MFServo::moveTo(int absolute)
{
    int newValue = map(absolute, _mapRange[0], _mapRange[1], _mapRange[2], _mapRange[3]);
    if (_targetPos != newValue) {
        _targetPos = newValue;
        if (!_initialized) {
#ifdef ARDUINO_ARCH_RP2040
            _servo.attach(_pin, 544, 2400);
#else
            _servo.attach(_pin);
#endif
            _initialized = true;
        }
    }
}

void MFServo::update()
{
    // after reaching final position
    // detach the servo to prevent continuous noise
    if (_currentPos == _targetPos) {
        // detach();
        return;
    }

    if (_currentPos > _targetPos)
        _currentPos--;
    else
        _currentPos++;

    _servo.write(_currentPos);
}

void MFServo::detach()
{
    if (_initialized) {
        _servo.detach();
        _initialized = false;
    }
}

void MFServo::attach(uint8_t pin, bool enable)
{
    _initialized = false;
    _targetPos   = 0;
    _currentPos  = 0;
    setExternalRange(0, 180);
    setInternalRange(0, 180);
    _pin = pin;
}

MFServo::MFServo()
    : _servo() {}

void MFServo::setExternalRange(int min, int max)
{
    _mapRange[0] = min;
    _mapRange[1] = max;
}

void MFServo::setInternalRange(int min, int max)
{
    _mapRange[2] = min;
    _mapRange[3] = max;
}

// MFServo.h
