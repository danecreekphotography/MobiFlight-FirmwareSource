//
// MFInputShifter.cpp
//
// (C) MobiFlight Project 2022
//

#include "MFInputShifter.h"
#include "allocateMem.h"

inputShifterEvent MFInputShifter::_inputHandler = NULL;

MFInputShifter::MFInputShifter()
{
    _initialized = false;
}

// Registers a new input shifter and configures the clock, data and latch pins as well
// as the number of modules to read from.
bool MFInputShifter::attach(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t moduleCount, const char *name)
{
    _latchPin    = latchPin;
    _clockPin    = clockPin;
    _dataPin     = dataPin;
    _name        = name;
    _moduleCount = moduleCount;

    pinMode(_latchPin, OUTPUT);
    pinMode(_clockPin, OUTPUT);
    pinMode(_dataPin, INPUT);

    if (!FitInMemory(sizeof(uint8_t) * _moduleCount))
        return false;

    _inputBuffer = new (allocateMemory(sizeof(uint8_t) * _moduleCount)) uint8_t;
    for (uint8_t i = 0; i < _moduleCount; i++) {
        _inputBuffer[i] = 0;
    }
    _initialized = true;

    // And now initialize all buttons with the actual status
    poll(DONT_TRIGGER);

    return true;
}

void MFInputShifter::detach()
{
    _initialized = false;
}

// Reads the values from the attached modules, compares them to the previously
// read values, and calls the registered event handler for any inputs that
// changed from the previously read state.
void MFInputShifter::update()
{
    poll(DO_TRIGGER);
}

void MFInputShifter::poll(uint8_t doTrigger)
{
    digitalWrite(_clockPin, HIGH); // Preset clock to retrieve first bit
    digitalWrite(_latchPin, HIGH); // Disable input latching and enable shifting

    // Multiple chained modules are handled one at a time. As shiftIn() keeps getting
    // called it will pull in the data from each chained module.
    for (uint8_t module = 0; module < _moduleCount; module++) {
        uint8_t currentState;

        currentState = shiftIn(_dataPin, _clockPin, MSBFIRST);

        // If an input changed on the current module from the last time it was read
        // then hand it off to figure out which bits specifically changed.
        if (currentState != _inputBuffer[module]) {
            if (doTrigger) detectChanges(_inputBuffer[module], currentState, module);
            _inputBuffer[module] = currentState;
        }
    }

    digitalWrite(_latchPin, LOW); // disable shifting and enable input latching
}

// Detects changes between the current state and the previously saved state
// of a byte's worth of input.
void MFInputShifter::detectChanges(uint8_t lastState, uint8_t currentState, uint8_t module)
{
    for (uint8_t i = 0; i < 8; i++) {
        // If last and current input state for the bit are different
        // then the input changed and the handler for the bit needs to fire
        if ((lastState & 1) ^ (currentState & 1)) {
            // When triggering event the pin is the actual pin on the chip offset by 8 bits for each
            // module beyond the first that it's on. The state of the trigger is the bit currently
            // in position 0 of currentState.
            trigger(i + (module * 8), currentState & 1);
        }

        lastState    = lastState >> 1;
        currentState = currentState >> 1;
    }
}

// Triggers the event handler for the associated input shift register pin,
// if a handler is registered.
void MFInputShifter::trigger(uint8_t pin, bool state)
{
    (*_inputHandler)((state == LOW ? inputShifterOnPress : inputShifterOnRelease), pin, _name);
}

void MFInputShifter::triggerOnPress()
{
    if (!_initialized || !_inputHandler)
        return;

    uint8_t state;

    poll(DONT_TRIGGER);

    // Trigger all the pressed buttons
    for (int module = 0; module < _moduleCount; module++) {
        state = _inputBuffer[module];
        for (uint8_t i = 0; i < 8; i++) {
            // Only trigger if the button is in the on position
            if (!(state & 1)) {
                trigger(i + (module * 8), LOW);
            }

            state = state >> 1;
        }
    }
}

void MFInputShifter::triggerOnRelease()
{
    if (!_initialized || !_inputHandler)
        return;

    uint8_t state;

    poll(DONT_TRIGGER);

    // Trigger all the released buttons
    for (int module = 0; module < _moduleCount; module++) {
        state = _inputBuffer[module];
        for (uint8_t i = 0; i < 8; i++) {
            // Only trigger if the button is in the off position
            if (state & 1) {
                trigger(i + (module * 8), HIGH);
            }
            state = state >> 1;
        }
    }
}

// Attaches a new event handler for the specified event.
void MFInputShifter::attachHandler(inputShifterEvent newHandler)
{
    _inputHandler = newHandler;
}

// MFInputShifter.cpp
