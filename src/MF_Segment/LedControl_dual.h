//
// LedControl_dual.cpp
//
// (C) MobiFlight Project 2023
//
// @author GiorgioCC (g.crocic@gmail.com) - 2023-06-29
//
// Remarks:
//
// Portions of code derived from:
// - LedControl - A library for controlling Leds with a MAX7219/MAX7221
//   Copyright (c) 2007 Eberhard Fahle
// - TM1637TinyDisplay - TM1637 Tiny Display library by Jason A. Cox
//   (https://github.com/jasonacox)
// =======================================================================
// This is basically a mix of two drivers in the same library (with common
// code parts factorized as much as possible):
// the type of driver is determined by the first argument in constructor
// =======================================================================
// Method signatures (and purpose) have been kept exactly as in the original
// LedControl library used in MF firmware, so the interface doesn't change;
// non-relevant arguments (particularly: <addr> for TM's) are ignored.
// A few methods (mostly for internal use) have been added.

#pragma once

// This constant reduces buffer usage: a single (static) 16-byte buffer
// is used for all objects. However, for TM1637s, data is written
// byte-by-byte, making transmission of data blocks slower.
// #define LEDCONTROL_NO_BUF

// This constant adds methods to print a decimal/hex number or a string
// (as opposite to writing individual chars).
#define LEDCONTROL_EXTENDED

#include <Arduino.h>
#include <LedSegment.h>

#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#define pgm_read_byte(addr) \
    (*(const unsigned char *)(addr)) // workaround for non-AVR
#endif

// =======================================================================
// TM1637 Definitions
// =======================================================================

#define DEFAULT_BIT_DELAY 100
#define TYPE_UNDEFINED    0xFF
#define MAX_BRIGHTNESS    15

// =======================================================================

class LedControl
{
private:
    // Common
    uint8_t _type    = TYPE_UNDEFINED;
    uint8_t _dataPin = TYPE_UNDEFINED;
    uint8_t _clkPin  = TYPE_UNDEFINED;
    uint8_t _csPin   = TYPE_UNDEFINED;
#if defined(LEDCONTROL_NO_BUF)
    // For TM, buffer can't be static (= shared): either we are building
    // the extended version (which adds a per-unit buffer instead of the static one)
    // or we are forced to resort to digit-by-digit output
    static uint8_t *rawdata;
#else
    uint8_t *rawdata;
#endif

    uint8_t maxUnits   = 0; // MAX: N. of chained units; TM: N. of digits
    uint8_t brightness = MAX_BRIGHTNESS;
    void    setPattern(uint8_t addr, uint8_t digit, uint8_t value, bool sendNow = true);

    // MAX-specific
    uint8_t *digitBuffer; // each digit must be stored in a buffer to be able to set single segments
    void     setScanLimit(uint8_t addr, uint8_t limit);
    void     spiTransfer(uint8_t addr, uint8_t opcode, uint8_t data);

    // TM-specific
    // uint8_t dpSet = 0;
    void bitDelay() { delayMicroseconds(DEFAULT_BIT_DELAY); };
    void start(void);
    void stop(void);
    bool writeByte(uint8_t data, bool rvs = false);

#if defined(LEDCONTROL_NO_BUF)
    void writeOneDigit(uint8_t ndigit, uint8_t val);
#else
    // Has buffer available
    void writeDigits(uint8_t ndigit, uint8_t len);
    void writeBuffer(void) { writeDigits(maxUnits - 1, maxUnits); };
#endif

public:
    LedControl() {};

    bool begin(uint8_t type, uint8_t dataPin, uint8_t clkPin, uint8_t csPin, uint8_t numDevices = 1);

    bool    isMAX(void) { return _type == LedSegment::TYPE_MAX72XX; }
    uint8_t getDeviceCount(void) { return (isMAX() ? maxUnits : 1); };
    uint8_t getDigitCount(void) { return (isMAX() ? 8 : maxUnits); };

    void shutdown(uint8_t addr, bool status);
    void setIntensity(uint8_t addr, uint8_t intensity);
    void clearDisplay(uint8_t addr = 0);

    // Display a hexadecimal digit.
    // Params:
    // addr	    address of the display (ignored for TM)
    // digit	the position of the digit on the display (0 is RIGHTMOST)
    // value	the value to be displayed. (0x00..0x0F)
    // dp	    sets the decimal point.
    // sendnow  If false, buffers chars rather than sending them immediately (TM only;
    //          requires a sendAll() afterwards).
    //          Ignored for MAX, or if LEDCONTROL_NO_BUF is defined.
    void setDigit(uint8_t addr, uint8_t digit, uint8_t value, bool dp = false, bool sendNow = true);

    // Display a character.
    // There are only a few characters that make sense here :
    //'0','1','2','3','4','5','6','7','8','9','0',
    //  'A','b','c','d','E','F','H','L','P',
    //  '.','-','_',' '
    // Params:
    // addr	    address of the display (ignored for TM)
    // digit	the position of the character on the display (0 is RIGHTMOST)
    // value	the character to be displayed.
    // dp	    sets the decimal point.
    // sendnow  If false, buffers chars rather than sending them immediately (TM only;
    //          requires a sendAll() afterwards).
    //          Ignored for MAX, or if LEDCONTROL_NO_BUF is defined.
    void setChar(uint8_t addr, uint8_t digit, char value, bool dp = false, bool sendNow = true);

    // Display a Single Segment.
    // Params:
    // addr	    address of the display (ignored for TM)
    // segment	the segment to be set or unset
    // value	set or unset the Segment
    // sendnow  If false, buffers chars rather than sending them immediately (TM only;
    //          requires a sendAll() afterwards).
    //          Ignored for MAX, or if LEDCONTROL_NO_BUF is defined.
    void setSingleSegment(uint8_t addr, uint8_t segment, uint8_t value, bool sendNow = true);

#ifndef LEDCONTROL_NO_BUF
    // Sends the whole (previously filled) buffer content.
    void sendAll(void) { writeBuffer(); };
#endif

#if defined(LEDCONTROL_EXTENDED)
    // Display a decimal number, with dot control
    //
    // Display the given argument as a decimal number. The dots between the digits
    // can be individually controlled.
    //
    // @param addr  The number of the chained device (MAX only, unused for TM)
    // @param num   The number to be shown
    // @param isHex <true> for Hex representation
    // @param dots Dot/Colon enable. The argument is a bitmask, with each bit corresponding to a dot
    //        between the digits (RIGHT aligned).
    // @param leading_zero When true, leading zeros are displayed. Otherwise unnecessary digits are
    //        blank.
    // @param rstart The position of the LEAST significant digit (N-1 = leftmost, 0 = rightmost)
    void showNumber(uint8_t addr, int32_t num, bool isHex = false, uint8_t dots = 0,
                    bool leading_zero = false, uint8_t roffset = 0);

    // Display a string
    //
    // Display the given string and if more than 4 characters, will scroll message on display
    //
    // @param addr  The number of the chained device (MAX only, unused for TM)
    // @param s The string to be shown
    // @param lstart The position of the most significant digit (0 - leftmost, N-1 - rightmost)
    // @param dots Dot/Colon enable. The argument is a bitmask, with each bit corresponding to a dot
    //        between the digits (LEFT aligned)
    // See showString_P function for reading PROGMEM read-only flash memory space instead of RAM
    void showString(uint8_t addr, char *s, uint8_t loffset = 0, uint8_t dots = 0);
    // void    showString_P(uint8_t addr, const char s[], uint8_t pos = 0, uint8_t dots = 0);

#endif
};
