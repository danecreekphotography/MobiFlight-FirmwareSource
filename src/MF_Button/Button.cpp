//
// Button.cpp
//
// (C) MobiFlight Project 2022
//

#include "mobiflight.h"
#include "MFButton.h"
#include "Button.h"

namespace Button
{
    MFButton *buttons;
    uint8_t   buttonsRegistered = 0;
    uint8_t   maxButtons        = 0;

    void handlerButtonOnChange(uint8_t eventId, const char *name)
    {
        cmdMessenger.sendCmdStart(kButtonChange);
        cmdMessenger.sendCmdArg(name);
        cmdMessenger.sendCmdArg(eventId);
        cmdMessenger.sendCmdEnd();
    };

    bool setupArray(uint16_t count)
    {
        if (!FitInMemory(sizeof(MFButton) * count))
            return false;
        buttons    = new (allocateMemory(sizeof(MFButton) * count)) MFButton;
        maxButtons = count;
        return true;
    }

    void Add(uint8_t pin, char const *name)
    {
        if (buttonsRegistered == maxButtons)
            return;
        buttons[buttonsRegistered] = MFButton();
        buttons[buttonsRegistered].attach(pin, name);
        MFButton::attachHandler(handlerButtonOnChange);
        buttonsRegistered++;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Added button ") /* + name */);
#endif
    }

    void Clear(void)
    {
        for (uint8_t i = 0; i < buttonsRegistered; i++) {
            buttons[i].detach();
        }
        buttonsRegistered = 0;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Cleared buttons"));
#endif
    }

    void read(void)
    {
        for (uint8_t i = 0; i < buttonsRegistered; i++) {
            buttons[i].update();
        }
    }

    void OnTrigger(void)
    {
        // Trigger all button release events first...
        for (uint8_t i = 0; i < buttonsRegistered; i++) {
            buttons[i].triggerOnRelease();
        }
        // ... then trigger all the press events
        for (uint8_t i = 0; i < buttonsRegistered; i++) {
            buttons[i].triggerOnPress();
        }
    }
} // namespace button

// Button.cpp
