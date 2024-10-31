//
// Analog.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>

namespace Analog
{
    bool setupArray(uint16_t count);
    void Add(uint8_t pin, char const *name = "AnalogInput", uint8_t sensitivity = 3);
    void Clear();
    void read();
    void readAverage();
    void OnTrigger();
}

// Analog.h