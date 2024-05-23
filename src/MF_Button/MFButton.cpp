//
// MFButton.cpp
//
// (C) MobiFlight Project 2022
//
#include "MFButton.h"

buttonEvent MFButton::_inputHandler = NULL;

MFButton::MFButton()
{
    _initialized = false;
}

void MFButton::attach(uint8_t pin, const char *name)
{
    _pin  = pin;
    _name = name;
    pinMode(_pin, INPUT_PULLUP);      // set pin to input
    _state       = digitalRead(_pin); // initialize on actual status
    _initialized = true;
}

void MFButton::detach()
{
    _initialized = false;
}

void MFButton::update()
{
    if (!_initialized)
        return;
    uint8_t newState = (uint8_t)digitalRead(_pin);
    if (newState != _state) {
        _state = newState;
        trigger(_state);
    }
}

void MFButton::trigger(uint8_t state)
{
    (state == LOW) ? triggerOnPress() : triggerOnRelease();
}

void MFButton::triggerOnPress()
{
    if (!_initialized)
        return;
    if (_inputHandler && _state == LOW) {
        (*_inputHandler)(btnOnPress, _name);
    }
}

void MFButton::triggerOnRelease()
{
    if (!_initialized)
        return;
    if (_inputHandler && _state == HIGH) {
        (*_inputHandler)(btnOnRelease, _name);
    }
}

void MFButton::attachHandler(buttonEvent newHandler)
{
    _inputHandler = newHandler;
}

// MFButton.cpp