//
// MFInputShifter.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>

extern "C" {
typedef void (*inputShifterEvent)(byte, uint8_t, uint8_t);
};

enum {
    inputShifterOnPress,
    inputShifterOnRelease,
};

class MFInputShifter
{
public:
    MFInputShifter();
    bool        attach(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t moduleCount, uint8_t deviceID);
    static void attachHandler(inputShifterEvent newHandler);
    void        detach();
    void        update();
    void        triggerOnPress();
    void        triggerOnRelease();

private:
    enum { DONT_TRIGGER = 0,
           DO_TRIGGER   = 1 };

    uint8_t  _deviceID;
    uint8_t  _latchPin;    // SH/~LD (latch) pin
    uint8_t  _clockPin;    // CLK (clock) pin
    uint8_t  _dataPin;     // SDO (data) pin
    uint8_t  _moduleCount; // Number of 8 bit modules in series.
    bool     _initialized = false;
    uint8_t *_lastState;

    void poll(uint8_t doTrigger);
    void detectChanges(uint8_t lastState, uint8_t currentState, uint8_t module);
    void trigger(uint8_t pin, bool state);

    static inputShifterEvent _inputHandler;
};

// MFInputShifter.h
