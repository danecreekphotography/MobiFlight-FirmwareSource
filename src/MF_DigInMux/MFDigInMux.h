//
// MFDigInMux.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>
#include "MFMuxDriver.h"

extern "C" {
typedef void (*MuxDigInEvent)(byte, uint8_t, uint8_t);
};

enum {
    MuxDigInOnPress,
    MuxDigInOnRelease,
};

class MFDigInMux
{
public:
    MFDigInMux(void);
    MFDigInMux(MFMuxDriver *MUX);
    static void setMux(MFMuxDriver *MUX);
    static void attachHandler(MuxDigInEvent newHandler);

    void     attach(uint8_t dataPin, bool halfSize, uint8_t deviceID);
    void     detach();
    void     clear();
    void     retrigger();
    void     update();
    uint16_t getValues(void) { return _lastState; }

private:
    enum { MUX_INITED   = 0,
           MUX_HALFSIZE = 1,
    };

    enum { DONT_TRIGGER = 0,
           DO_TRIGGER   = 1 };

    static MFMuxDriver  *_MUX;
    static MuxDigInEvent _inputHandler;

    uint8_t  _deviceID;
    uint8_t  _dataPin; // Data pin - MUX common, input to AVR
    uint8_t  _flags;
    uint16_t _lastState;

    void poll(bool detect);
    void detectChanges(uint16_t lastState, uint16_t currentState);
    void trigger(uint8_t channel, bool state);
};

// MFDigInMux.h