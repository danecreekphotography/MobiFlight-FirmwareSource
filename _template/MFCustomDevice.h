#pragma once

#include <Arduino.h>
#include "MyCustomClass.h"

// only one entry required if you have only one custom device
enum {
    MY_CUSTOM_DEVICE_1 = 1,
    MY_CUSTOM_DEVICE_2
};

bool MFCustomDeviceGetConfig();

class MFCustomDevice
{
public:
    MFCustomDevice();
    void attach(uint16_t adrPin, uint16_t adrType, uint16_t adrConfig);
    void detach();
    void update();
    void set(int16_t messageID, char *setPoint);

private:
    bool           getStringFromEEPROM(uint16_t addreeprom, char *buffer);
    bool           _initialized = false;
    MyCustomClass *_mydevice;
    uint8_t        _pin1, _pin2, _pin3;
    uint8_t        _customType = 0;
};
