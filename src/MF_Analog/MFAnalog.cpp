//
// MFAnalog.cpp
//
// (C) MobiFlight Project 2022
//

#include "MFAnalog.h"

analogEvent MFAnalog::_handler = NULL;

MFAnalog::MFAnalog()
{
    _initialized = false;
}

void MFAnalog::attach(uint8_t pin, const char *name, uint8_t sensitivity)
{
    _sensitivity = sensitivity;
    _pin         = pin;
    _name        = name;
#if defined(ARDUINO_AVR_PROMICRO16)
    // ProMicro has a special pin assignment for analog pins
    // therefore reading from A6 and A7 does not work
    // via "digital" pins. See also pins_arduino.h
    if (_pin == 4)
        _pin = A6;
    else if (_pin == 6)
        _pin = A7;
#endif
    pinMode(_pin, INPUT_PULLUP); // set pin to input. Could use OUTPUT for analog, but shows the intention :-)
    // Fill averaging buffers with initial reading
    for (uint8_t i = 0; i < ADC_MAX_AVERAGE; i++) {
        readBuffer();
    }
    // and set initial value from buffers
    _lastValue = ADC_Average_Total >> ADC_MAX_AVERAGE_LOG2;
    _initialized = true;
}

bool MFAnalog::valueHasChanged(int16_t newValue)
{
    if (!_initialized)
        return false;
    return (abs(newValue - _lastValue) >= _sensitivity);
}

void MFAnalog::readChannel(uint8_t alwaysTrigger)
{
    if (!_initialized)
        return;
    int16_t newValue = ADC_Average_Total >> ADC_MAX_AVERAGE_LOG2;
    if (alwaysTrigger || valueHasChanged(newValue)) {
        _lastValue = newValue;
        if (_handler != NULL) {
            (*_handler)(_lastValue, _name);
        }
    }
}

void MFAnalog::update()
{
    readChannel(false);
}

void MFAnalog::retrigger()
{
    readChannel(true);
}

void MFAnalog::readBuffer()
{                          
    if (!_initialized)
        return;                                 // read ADC and calculate floating average, call it every ~10ms
    ADC_Average_Total -= ADC_Buffer[(ADC_Average_Pointer)]; // subtract oldest value to save the newest value
    ADC_Buffer[ADC_Average_Pointer] = analogRead(_pin);     // store read in, must be subtracted in next loop
    ADC_Average_Total += ADC_Buffer[ADC_Average_Pointer];   // add read in for floating average
    ADC_Average_Pointer++;                                  // prepare for next loop
    ADC_Average_Pointer &= (ADC_MAX_AVERAGE - 1);           // limit max. values for floating average
}

void MFAnalog::attachHandler(analogEvent newHandler)
{
    _handler = newHandler;
}

// MFAnalog.cpp