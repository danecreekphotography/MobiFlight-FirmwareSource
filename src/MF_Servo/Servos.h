//
// Servos.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>

namespace Servos
{
    bool setupArray(uint16_t count);
    void Add(uint8_t pin);
    void Clear();
    void OnSet();
    void update();
}

// Servos.h