//
// Button.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <stdint.h>

namespace Button
{
    bool setupArray(uint16_t count);
    void Add(uint8_t pin);
    void Clear();
    void read();
    void OnTrigger();
}

// Button.h