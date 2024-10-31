//
// OutputShifter.cpp
//
// (C) MobiFlight Project 2022
//

#include "commandmessenger.h"
#include "allocateMem.h"
#include "MFOutputShifter.h"
#include "OutputShifter.h"

namespace OutputShifter
{
    MFOutputShifter *outputShifter;
    uint8_t          outputShifterRegistered = 0;
    uint8_t          maxOutputShifter        = 0;

    bool setupArray(uint16_t count)
    {
        if (!FitInMemory(sizeof(MFOutputShifter) * count))
            return false;
        outputShifter    = new (allocateMemory(sizeof(MFOutputShifter) * count)) MFOutputShifter;
        maxOutputShifter = count;
        return true;
    }

    void Add(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t modules)
    {
        if (outputShifterRegistered == maxOutputShifter)
            return;
        outputShifter[outputShifterRegistered] = MFOutputShifter();
        if (!outputShifter[outputShifterRegistered].attach(latchPin, clockPin, dataPin, modules)) {
            cmdMessenger.sendCmd(kStatus, F("OutputShifter array does not fit into Memory"));
            return;
        }
        outputShifterRegistered++;

#if defined(DEBUG2CMDMESSENGER)
        cmdMessenger.sendCmd(kDebug, F("Added Output Shifter"));
#endif
    }

    void Clear()
    {
        for (uint8_t i = 0; i < outputShifterRegistered; i++) {
            outputShifter[i].detach();
        }

        outputShifterRegistered = 0;
#if defined(DEBUG2CMDMESSENGER)
        cmdMessenger.sendCmd(kDebug, F("Cleared Output Shifter"));
#endif
    }

    void OnSet()
    {

        int   module = cmdMessenger.readInt16Arg();
        char *pins   = cmdMessenger.readStringArg();
        int   value  = cmdMessenger.readInt16Arg();
        outputShifter[module].setPins(pins, value);
    }

    void PowerSave(bool state)
    {
        for (uint8_t i = 0; i < outputShifterRegistered; ++i) {
            outputShifter[i].powerSavingMode(state);
        }
    }
} // namespace

// OutputShifter.cpp
