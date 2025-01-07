//
// LCDDisplay.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>

namespace LCDDisplay
{
    bool setupArray(uint16_t count);
    void Add(uint8_t address = 0x24, uint8_t cols = 16, uint8_t lines = 2);
    void Clear();
    void OnSet();
    void PowerSave(bool state);
}

// LCDDisplay.h
