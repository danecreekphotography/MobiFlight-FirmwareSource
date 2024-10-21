//
// OutputShifter.h
//
// (C) MobiFlight Project 2022
//

#pragma once

namespace OutputShifter
{
    bool setupArray(uint16_t count);
    void Add(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t modules);
    void Clear();
    void OnSet();
    void PowerSave(bool state);
}

// OutputShifter.h