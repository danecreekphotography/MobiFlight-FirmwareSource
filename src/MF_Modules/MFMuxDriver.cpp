//
// MFMuxDriver.cpp
//
// (C) MobiFlight Project 2022
//

#include "mobiflight.h"
#include "MFMuxDriver.h"

MFMuxDriver::MFMuxDriver(void)
{
    _flags = 0x00;
    for (uint8_t i = 0; i < 4; i++) {
        _selPin[i] = 0xFF;
    }
}

// Registers a new MUX input block and configures the driver pins
void MFMuxDriver::
    attach(uint8_t Sel0Pin, uint8_t Sel1Pin, uint8_t Sel2Pin, uint8_t Sel3Pin)
{
#ifdef USE_FAST_IO
    _selPinFast[0].Port = portOutputRegister(digitalPinToPort(Sel0Pin));
    _selPinFast[0].Mask = digitalPinToBitMask(Sel0Pin);
    _selPinFast[1].Port = portOutputRegister(digitalPinToPort(Sel1Pin));
    _selPinFast[1].Mask = digitalPinToBitMask(Sel1Pin);
    _selPinFast[2].Port = portOutputRegister(digitalPinToPort(Sel2Pin));
    _selPinFast[2].Mask = digitalPinToBitMask(Sel2Pin);
    _selPinFast[3].Port = portOutputRegister(digitalPinToPort(Sel3Pin));
    _selPinFast[3].Mask = digitalPinToBitMask(Sel3Pin);
#endif
    _selPin[0] = Sel0Pin;
    _selPin[1] = Sel1Pin;
    _selPin[2] = Sel2Pin;
    _selPin[3] = Sel3Pin;
    _flags     = 0x00;

    for (uint8_t i = 0; i < 4; i++)
        pinMode(_selPin[i], OUTPUT);
    bitSet(_flags, MUX_INITED);

    setChannel(0);
}

void MFMuxDriver::detach()
{
    for (uint8_t i = 0; i < 4; i++) {
        if (_selPin[i] != 0xFF) {
            pinMode(_selPin[i], INPUT_PULLUP);
            _selPin[i] = 0xFF;
        }
    }
    bitClear(_flags, MUX_INITED);
}

// Sets the driver lines to select the specified channel
void MFMuxDriver::setChannel(uint8_t value)
{
    if (!bitRead(_flags, MUX_INITED)) return;
    if (value > 15) return;

    // Ideally, setChannel() should change all pins atomically (at the same time):
    // since it doesn't, be advised that there will be signal glitches because
    // the actual code - which is not latched - spans several values as the single bits are changed.
    // This should not be an issue, because e.g. in an input mux the output is only read at the end,
    // once the code is stable.
    // (Please note that output value settling is a completely different effect.)
    // However, this effect might have to be taken into account.

    _channel = value;
    for (uint8_t i = 0; i < 4; i++) {
#ifdef USE_FAST_IO
        DIGITALWRITE(_selPinFast[i], (value & 0x01));
#else
        DIGITALWRITE(_selPin[i], (value & 0x01));
#endif
        value >>= 1;
    }
}

// Returns currently selected channel
uint8_t MFMuxDriver::getChannel(void)
{
    return _channel;
}

// Increments current channel, wraps around to 0
uint8_t MFMuxDriver::nextChannel(void)
{
    setChannel((++_channel) % 16);
    return _channel;
}

// Temporarily stores current channel for later retrieval
void MFMuxDriver::saveChannel(void)
{
    _savedChannel = _channel;
}

// Restored previously stored channel
void MFMuxDriver::restoreChannel(void)
{
    setChannel(_savedChannel);
}

// MFMuxDriver.cpp
