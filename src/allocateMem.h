//
// allocatemem.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>
#include <new>

#if defined (ARDUINO_ARCH_AVR)
uint8_t     *allocateMemory(uint16_t size);
#else
std::size_t    *allocateMemory(uint16_t size);
#endif

void        ClearMemory();
uint16_t    GetAvailableMemory();
bool        FitInMemory(uint16_t size);

// allocatemem.h
