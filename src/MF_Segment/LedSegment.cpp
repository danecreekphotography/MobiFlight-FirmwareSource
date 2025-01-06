//
// LedSegment.cpp
//
// (C) MobiFlight Project 2022
//

#include "mobiflight.h"
#include "MFSegments.h"
#include "LedSegment.h"

namespace LedSegment
{
    MFSegments *ledSegments;
    uint8_t     ledSegmentsRegistered  = 0;
    uint8_t     ledSegmentsRegistereds = 0;

    bool setupArray(uint16_t count)
    {
        if (!FitInMemory(sizeof(MFSegments) * count))
            return false;
        ledSegments            = new (allocateMemory(sizeof(MFSegments) * count)) MFSegments;
        ledSegmentsRegistereds = count;
        return true;
    }

    void Add(uint8_t type, uint8_t dataPin, uint8_t csPin, uint8_t clkPin, uint8_t numDevices, uint8_t brightness)
    {
        if (ledSegmentsRegistered == ledSegmentsRegistereds)
            return;

        ledSegments[ledSegmentsRegistered] = MFSegments();

        if (!ledSegments[ledSegmentsRegistered].attach(type, dataPin, csPin, clkPin, numDevices, brightness))
        {
            cmdMessenger.sendCmd(kStatus, F("Led Segment array does not fit into Memory"));
            return;
        }

        ledSegmentsRegistered++;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmdStart(kDebug);
        cmdMessenger.sendCmdArg(F("Added Led Segment with "));
        cmdMessenger.sendCmdArg(sizeof(MFSegments));
        cmdMessenger.sendCmdArg(F(" byte"));
        cmdMessenger.sendCmdEnd();
#endif
    }

    void Clear()
    {
        for (uint8_t i = 0; i < ledSegmentsRegistered; i++) {
            ledSegments[i].detach();
        }
        ledSegmentsRegistered = 0;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Cleared segments"));
#endif
    }

    void PowerSave(bool state)
    {
        for (uint8_t i = 0; i < ledSegmentsRegistered; ++i) {
            ledSegments[i].powerSavingMode(state);
        }
    }

    void OnInitModule()
    {
        int module     = cmdMessenger.readInt16Arg();
        int subModule  = cmdMessenger.readInt16Arg();
        int brightness = cmdMessenger.readInt16Arg();
        ledSegments[module].setBrightness(subModule, brightness);
    }

    void OnSetModule()
    {
        int     module    = cmdMessenger.readInt16Arg();
        int     subModule = cmdMessenger.readInt16Arg();
        char   *value     = cmdMessenger.readStringArg();
        uint8_t points    = (uint8_t)cmdMessenger.readInt16Arg();
        uint8_t mask      = (uint8_t)cmdMessenger.readInt16Arg();
        ledSegments[module].display(subModule, value, points, mask);
    }

    void OnSetModuleBrightness()
    {
        int module     = cmdMessenger.readInt16Arg();
        int subModule  = cmdMessenger.readInt16Arg();
        int brightness = cmdMessenger.readInt16Arg();
        ledSegments[module].setBrightness(subModule, brightness);
    }

    void OnSetModuleSingleSegment()
    {
        uint8_t module     = (uint8_t)cmdMessenger.readInt16Arg();
        uint8_t subModule  = (uint8_t)cmdMessenger.readInt16Arg();
        char *segment      = cmdMessenger.readStringArg();              // 0 to 63, multiple segments deliminited by '|'
        uint8_t on_off     = (uint8_t)cmdMessenger.readInt16Arg();      // 0 or 1

        char *pinTokens = strtok(segment, "|");
        while (pinTokens != 0) {
            uint8_t num = (uint8_t)atoi(pinTokens);
            ledSegments[module].setSingleSegment(subModule, num, on_off);
            pinTokens = strtok(0, "|");
        }
    }
    
} // namespace

// LedSegment.cpp
