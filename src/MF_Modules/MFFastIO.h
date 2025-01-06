//
// MFFastIO.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>

#ifdef USE_FAST_IO

#define DIGITALREAD(a)     digitalReadFast(a)
#define DIGITALWRITE(a, b) digitalWriteFast(a, b)
typedef struct {
#ifdef ARDUINO_ARCH_RP2040
    volatile uint32_t *Port;
    uint32_t           Mask;
#else
    volatile uint8_t *Port;
    uint8_t           Mask;
#endif
} FASTIO_s;

inline void digitalWriteFast(FASTIO_s Pin, uint8_t value)
{
    if (value)
        *Pin.Port |= Pin.Mask;
    else
        *Pin.Port &= ~Pin.Mask;
}

inline uint8_t digitalReadFast(FASTIO_s Pin)
{
    if (*Pin.Port & Pin.Mask) return HIGH;
    return LOW;
}

#else

#define DIGITALREAD(a)     digitalRead(a)
#define DIGITALWRITE(a, b) digitalWrite(a, b)

typedef uint8_t FASTIO_s;

#endif
