//
// MFButton.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>
#include "MFFastIO.h"

extern "C" {
// callback functions always follow the signature: void cmd(void);
typedef void (*buttonEvent)(uint8_t, const char *);
};

enum {
    btnOnPress,
    btnOnRelease,
};

/////////////////////////////////////////////////////////////////////
/// \class MFButton MFButton.h <MFButton.h>
class MFButton
{
public:
    MFButton();
    static void attachHandler(buttonEvent newHandler);
    void        attach(uint8_t pin, const char *name);
    void        detach();
    void        update();
    void        trigger(uint8_t state);
    void        triggerOnPress();
    void        triggerOnRelease();

private:
    const char *_name;
    FASTIO_s    _pin;
    bool        _initialized;
    bool        _state;

    static buttonEvent _inputHandler;
};

// MFButton.h