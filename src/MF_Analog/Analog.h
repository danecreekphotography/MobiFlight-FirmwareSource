//
// Analog.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <stdint.h>
namespace Analog
{
    bool setupArray(uint16_t count);
    void Add(uint8_t pin, uint8_t sensitivity = 3);
    void Clear();
    void read();
    void readAverage();
    void OnTrigger();
}

// Analog.h