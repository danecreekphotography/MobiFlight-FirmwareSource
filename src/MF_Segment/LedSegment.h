//
// LedSegment.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>

namespace LedSegment
{
    enum {
        TYPE_MAX72XX        = 0,
        TYPE_TM1637_4DIGITS = 0xFD,
        TYPE_TM1637_6DIGITS = 0xFE,
        TYPE_UNDEFINED      = 0xFF
    };

    bool setupArray(uint16_t count);
    void Add(uint8_t type, uint8_t dataPin, uint8_t csPin, uint8_t clkPin, uint8_t numDevices, uint8_t brightness);
    void Clear();
    void PowerSave(bool state);
    void OnInitModule();
    void OnSetModule();
    void OnSetModuleBrightness();
    void OnSetModuleSingleSegment();
}

// LedSegment.h
