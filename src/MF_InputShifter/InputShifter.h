//
// InputShifter.h
//
// (C) MobiFlight Project 2022
//

#pragma once

namespace InputShifter
{
    bool setupArray(uint16_t count);
    void Add(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t modules, char const *name = "InputShifter");
    void Clear();
    void read();
    void OnTrigger();
}

// InputShifter.h