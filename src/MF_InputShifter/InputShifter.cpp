//
// InputShifter.cpp
//
// (C) MobiFlight Project 2022
//

#include "config.h"
#include "commandmessenger.h"
#include "allocateMem.h"
#include "MFInputShifter.h"
#include "InputShifter.h"

namespace InputShifter
{
    MFInputShifter *inputShifter;
    uint8_t         inputShifterRegistered = 0;
    uint8_t         maxInputShifter        = 0;

    void handlerInputShifterOnChange(uint8_t eventId, uint8_t pin, const char *name)
    {
        if (!getBoardReady())
            return;
        cmdMessenger.sendCmdStart(kInputShifterChange);
        cmdMessenger.sendCmdArg(name);
        cmdMessenger.sendCmdArg(pin);
        cmdMessenger.sendCmdArg(eventId);
        cmdMessenger.sendCmdEnd();
    };

    bool setupArray(uint16_t count)
    {
        if (!FitInMemory(sizeof(MFInputShifter) * count))
            return false;
        inputShifter    = new (allocateMemory(sizeof(MFInputShifter) * count)) MFInputShifter;
        maxInputShifter = count;
        return true;
    }

    void Add(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t modules, char const *name)
    {
        if (inputShifterRegistered == maxInputShifter)
            return;
        inputShifter[inputShifterRegistered] = MFInputShifter();
        if (!inputShifter[inputShifterRegistered].attach(latchPin, clockPin, dataPin, modules, name)) {
            cmdMessenger.sendCmd(kStatus, F("InputShifter array does not fit into Memory"));
            return;
        }
        MFInputShifter::attachHandler(handlerInputShifterOnChange);
        inputShifterRegistered++;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Added input shifter"));
#endif
    }

    void Clear()
    {
        for (uint8_t i = 0; i < inputShifterRegistered; i++) {
            inputShifter[i].detach();
        }
        inputShifterRegistered = 0;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Cleared input shifter"));
#endif
    }

    void read()
    {
        for (uint8_t i = 0; i < inputShifterRegistered; i++) {
            inputShifter[i].update();
        }
    }

    void OnTrigger()
    {
        // Retrigger all the input shifters. This automatically sends
        // the release events first followed by press events.

        // Trigger all button release events first...
        for (uint8_t i = 0; i < inputShifterRegistered; i++) {
            inputShifter[i].triggerOnRelease();
        }
        // ... then trigger all the press events
        for (uint8_t i = 0; i < inputShifterRegistered; i++) {
            inputShifter[i].triggerOnPress();
        }
    }

} // namespace

// InputShifter.cpp
