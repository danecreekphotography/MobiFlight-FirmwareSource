//
// Analog.cpp
//
// (C) MobiFlight Project 2022
//

#include "mobiflight.h"
#include "MFAnalog.h"
#include "Analog.h"

#if MF_ANALOG_SUPPORT == 1
namespace Analog
{
    MFAnalog *analog;
    uint8_t   analogRegistered = 0;
    uint8_t   maxAnalogIn      = 0;

    void handlerOnAnalogChange(int value, uint8_t deviceID)
    {
        cmdMessenger.sendCmdStart(kAnalogChange);
        cmdMessenger.sendCmdArg(deviceID);
        cmdMessenger.sendCmdArg(value);
        cmdMessenger.sendCmdEnd();
    };

    bool setupArray(uint16_t count)
    {
        if (!FitInMemory(sizeof(MFAnalog) * count))
            return false;
        analog      = new (allocateMemory(sizeof(MFAnalog) * count)) MFAnalog();
        maxAnalogIn = count;
        return true;
    }

    void Add(uint8_t pin, uint8_t sensitivity)
    {
        if (analogRegistered == maxAnalogIn)
            return;

        analog[analogRegistered] = MFAnalog(pin, analogRegistered, sensitivity);
        MFAnalog::attachHandler(handlerOnAnalogChange);
        analogRegistered++;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Added analog device "));
#endif
    }

    void Clear(void)
    {
        analogRegistered = 0;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Cleared analog devices"));
#endif
    }

    void read(void)
    {
        for (uint8_t i = 0; i < analogRegistered; i++) {
            analog[i].update();
        }
    }

    void readAverage(void)
    {
        for (uint8_t i = 0; i < analogRegistered; i++) {
            analog[i].readBuffer();
        }
    }

    void OnTrigger()
    {
        // Scan and transit current values
        for (uint8_t i = 0; i < analogRegistered; i++) {
            analog[i].retrigger();
        }
    }

} // namespace Analog
#endif

// Analog.cpp
