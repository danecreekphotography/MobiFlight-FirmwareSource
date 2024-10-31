//
// Output.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>

namespace Output
{
    bool setupArray(uint16_t count);
    void Add(uint8_t pin = 1);
    void Clear();
    void OnSet();
    void PowerSave(bool state);
}

// Output.h
