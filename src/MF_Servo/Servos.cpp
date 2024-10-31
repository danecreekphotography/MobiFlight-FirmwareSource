//
// Servos.cpp
//
// (C) MobiFlight Project 2022
//

#include <Arduino.h>
#include "MFBoards.h"
#include "commandmessenger.h"
#include "allocateMem.h"
#include "MFServo.h"
#include "Servos.h"

namespace Servos
{
    MFServo *servos;
    uint8_t  servosRegistered = 0;
    uint8_t  maxServos        = 0;

    bool setupArray(uint16_t count)
    {
        if (!FitInMemory(sizeof(MFServo) * count))
            return false;
        servos    = new (allocateMemory(sizeof(MFServo) * count)) MFServo;
        maxServos = count;
        return true;
    }

    void Add(uint8_t pin)
    {
        if (servosRegistered == maxServos)
            return;
        servos[servosRegistered] = MFServo();
        servos[servosRegistered].attach(pin, true);
        servosRegistered++;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Added servos"));
#endif
    }

    void Clear()
    {
        for (uint8_t i = 0; i < servosRegistered; i++) {
            servos[i].detach();
        }
        servosRegistered = 0;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Cleared servos"));
#endif
    }

    void OnSet()
    {
        int servo    = cmdMessenger.readInt16Arg();
        int newValue = cmdMessenger.readInt16Arg();
        if (servo >= servosRegistered)
            return;
        servos[servo].moveTo(newValue);
    }

    void update()
    {
        for (uint8_t i = 0; i < servosRegistered; i++) {
            servos[i].update();
        }
    }
} // namespace

// Servos.cpp
