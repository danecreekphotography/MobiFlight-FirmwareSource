//
// MFEncoder.cpp
//
// (C) MobiFlight Project 2022
//

// Dased on the RotaryEncoder library from Matthias Hertel, see below
// Copyright (C) 2013-2014
// -----
// RotaryEncoder.cpp - Library for using rotary encoders.
// This class is implemented for use with the Arduino environment.
// Copyright (c) by Matthias Hertel, http://www.mathertel.de
// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
// More information on: http://www.mathertel.de/Arduino
// -----
// 18.01.2014 created by Matthias Hertel
// -----

#include "MFEncoder.h"

// The array holds the values -1 for the entries where a position was decremented,
// a 1 for the entries where the position was incremented
// and 0 in all the other (no change or not valid) cases.
const int8_t KNOBDIR[] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0};

// Configuration for different types of rotary encoders.
// For more information, refer to http://svglobe.com/arduino/in_encoder.html
//
// The detents in the encoder type settings below are indexed
// by value, not by the order in the quadrature cycle. For example, a rotary
// encoder with detents at 00 and 11 (positions 0 and 2 in the
// quadrature) but are indexed based on their decimal values (0 and 3).
// This allows simple lookup of detent positions by value.
const encoderType encoderTypes[]{

    // 1 detents per cycle:  00 ,  10 , [11],  01
    {{false, false, false, true}, 2},

    // 1 detents per cycle: [00],  10 ,  11 ,  01
    {{true, false, false, false}, 2},

    // 2 detents per cycle: [00],  10 , [11],  01
    {{true, false, false, true}, 1},

    // 2 detents per cycle:  00 , [10],  11,  [01]
    {{false, true, true, false}, 1},

    // 4 detents per cycle: [00], [10], [11], [01]
    {{true, true, true, true}, 0},
};

encoderEvent MFEncoder::_handler = NULL;

MFEncoder::MFEncoder()
{
    _initialized = false;
}

void MFEncoder::attach(uint8_t pin1, uint8_t pin2, uint8_t TypeEncoder, uint8_t deviceID)
{
    _pos         = 0;
    _deviceID    = deviceID;
    _pin1        = pin1;
    _pin2        = pin2;
    _encoderType = encoderTypes[TypeEncoder];

    pinMode(_pin1, INPUT_PULLUP);
    pinMode(_pin2, INPUT_PULLUP);
    // start with position 0;
    _oldState         = 0;
    _position         = 0;
    _positionExt      = 0;
    _detentCounter    = 0;
    _positionTimePrev = 0;   // for first startup avoid calculation of a high speed, _positionTimePrev
    _positionTime     = 500; // and _positionTime must be initialized to avoid that they have the same value
    _initialized      = true;
}

void MFEncoder::update()
{
    if (!_initialized)
        return;

    tick();

    int16_t pos = getPosition();
    if (pos == _pos) {
        // nothing happened
        return;
    }

    int16_t delta = pos - _pos;
    bool    dir   = true;

    if (delta < 0)
        dir = false;

    if (_handler) {
        if (abs(delta) < (MF_ENC_FAST_LIMIT)) {
            // slow turn detected
            if (dir) {
                (*_handler)(encLeft, _deviceID);
            } else {
                (*_handler)(encRight, _deviceID);
            }
        } else {
            // fast turn detected
            if (dir) {
                (*_handler)(encLeftFast, _deviceID);
            } else {
                (*_handler)(encRightFast, _deviceID);
            }
        }
    }

    // protect from overflow
    if ((dir && (pos + delta * 2) > MF_ENC_MAX) || (!dir && (pos - delta * 2) < -MF_ENC_MAX)) {
        setPosition(0);
        pos = 0;
    }
    _pos = pos;
}

void MFEncoder::tick(void)
{
    bool     sig1      = !digitalRead(_pin1); // to keep backwards compatibility for encoder type digitalRead must be negated
    bool     sig2      = !digitalRead(_pin2); // to keep backwards compatibility for encoder type digitalRead must be negated
    int      _speed    = 0;
    uint32_t currentMs = millis();

    int8_t thisState = sig1 | (sig2 << 1);

    if (currentMs - _lastFastDec > 100 && _detentCounter > 1) {
        _lastFastDec = currentMs;
        _detentCounter--;
    } else if (currentMs - _positionTimePrev > 500) { // if more than 80ms no step detected, set fast acceleration to 0
        _lastFastDec   = currentMs;
        _detentCounter = 0;
    }

    if (_oldState != thisState) {

        if (_detentCounter > 6) { // at minimum 6 detents have to be detected before fast step can be detected
            _speed = 51;
        } else {
            _speed = 1;
        }

        _position += ((KNOBDIR[thisState | (_oldState << 2)] * _speed)) << _encoderType.resolutionShift;
        if (_encoderType.detents[thisState]) {
            _positionTimePrev = _positionTime;
            _positionTime     = currentMs;
            _positionExt      = _position >> _encoderType.resolutionShift;
            _detentCounter    = min(_detentCounter + 1, 12);
        }
        _oldState = thisState;
    }
}

int16_t MFEncoder::getPosition()
{
    return _positionExt;
}

void MFEncoder::setPosition(int16_t newPosition)
{
    // only adjust the external part of the position.
    _position    = ((newPosition >> _encoderType.resolutionShift) | (_position & 0x03));
    _positionExt = newPosition;
}

void MFEncoder::attachHandler(encoderEvent newHandler)
{
    _handler = newHandler;
}

// MFEncoder.cpp
