#pragma once

#include <Arduino.h>

// If no input custom deivece is defined, uncomment the following line
// const char CustomDeviceConfig[] PROGMEM = {":"};

// Otherwise define your input custom devices
const char CustomDeviceConfig[] PROGMEM = 
{
    "1.2.Button Flash:"
    "8.3.4.0.Encoder Flash:"
    "11.54.5.Analog Input Flash:"
    "12.7.6.5.1.InputShifter Flash:"
    "14.12.8.9.10.11.2.Multiplexer Flash:"
};
