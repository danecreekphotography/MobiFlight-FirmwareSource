//
// MFStepper.cpp
//
// (C) MobiFlight Project 2022
//

#include "allocateMem.h"
#include "commandmessenger.h"
#include "MFStepper.h"

enum { // enumeration for stepper mode
    FULL4WIRE,
    HALF4WIRE,
    DRIVER
};

enum {
    MOVE_CCW,
    MOVE_CW
};

MFStepper::MFStepper()
{
    _initialized = false;
}

void MFStepper::attach(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t btnPin5, uint8_t mode, int8_t backlash, bool deactivateOutput)
{
    if (!FitInMemory(sizeof(AccelStepper))) {
        // Error Message to Connector
        cmdMessenger.sendCmd(kStatus, F("MFStepper does not fit in Memory"));
        return;
    }
    uint16_t maxSpeed = 0;
    uint16_t Accel    = 0;

    switch (mode) {
    case FULL4WIRE:
        maxSpeed = STEPPER_SPEED;
        Accel    = STEPPER_ACCEL;
        if (pin1 == pin3 && pin2 == pin4) // for backwards compatibility
            _stepper = new (allocateMemory(sizeof(AccelStepper))) AccelStepper(AccelStepper::DRIVER, pin1, pin2);
        else
            _stepper = new (allocateMemory(sizeof(AccelStepper))) AccelStepper(AccelStepper::FULL4WIRE, pin4, pin2, pin1, pin3);
        break;
    case HALF4WIRE:
        _stepper = new (allocateMemory(sizeof(AccelStepper))) AccelStepper(AccelStepper::HALF4WIRE, pin4, pin2, pin1, pin3);
        maxSpeed = STEPPER_SPEED;
        Accel    = STEPPER_ACCEL;
        break;
    case DRIVER:
        _stepper = new (allocateMemory(sizeof(AccelStepper))) AccelStepper(AccelStepper::DRIVER, pin1, pin2);
        maxSpeed = STEPPER_SPEED;
        Accel    = STEPPER_ACCEL;
        break;
    default:
        _initialized = false;
        return;
        break;
    }

    _stepper->setMaxSpeed(maxSpeed);
    _stepper->setAcceleration(Accel);
    _zeroPin      = btnPin5;
    _zeroPinState = HIGH;

    if (_zeroPin) {
        pinMode(_zeroPin, INPUT_PULLUP);
    }

    _backlash         = backlash;
    _deactivateOutput = deactivateOutput;
    _initialized      = true;
    _resetting        = false;
    _isStopped        = true;
    _inMove           = MOVE_CW;
}

void MFStepper::detach()
{
    _initialized = false;
}

void MFStepper::moveTo(long newPosition)
{
    if (!_initialized)
        return;

    _resetting           = false;
    long currentPosition = _stepper->currentPosition();

    if (_targetPos != newPosition) {
        if (_deactivateOutput && _isStopped) {
            _stepper->enableOutputs();
            _isStopped = false;
        }
        if (_inMove == MOVE_CW && newPosition < currentPosition && newPosition < _targetPos)
            _inMove = MOVE_CCW;
        if (_inMove == MOVE_CCW && newPosition > currentPosition && newPosition > _targetPos)
            _inMove = MOVE_CW;
        
        _stepper->moveTo(newPosition + _backlash * _inMove);
        _targetPos = newPosition;
    }
}

void MFStepper::setZero()
{
    if (!_initialized)
        return;

    _stepper->setCurrentPosition(0);
    if (_inMove == MOVE_CW) {
        _stepper->moveTo(-_backlash);
    }
}

void MFStepper::setZeroInReset()
{
    if (!_initialized)
        return;

    if (_resetting) {
        _stepper->setCurrentPosition(0);
        _resetting = false;
    }
}

void MFStepper::checkZeroPin()
{
    if (!_initialized)
        return;

    uint8_t newState = (uint8_t)digitalRead(_zeroPin);
    if (newState != _zeroPinState) {
        _zeroPinState = newState;
        if (_zeroPinState == LOW) setZeroInReset();
    }
}

void MFStepper::update()
{
    if (!_initialized)
        return;

    _stepper->run();
    checkZeroPin();
    if (_stepper->currentPosition() == (_targetPos + _backlash * _inMove) && _deactivateOutput) {
        _stepper->disableOutputs();
        _isStopped = true;
    }
}

void MFStepper::reset()
{
    if (!_initialized)
        return;

    // we are not a auto reset stepper if this pin is 0
    if (_zeroPin == 0)
        return;

    // if we are already resetting ignore next reset command
    if (_resetting)
        return;

    // flag that we are resetting
    _resetting = true;

    // tell stepper to move counter clockwise for a long while
    _stepper->moveTo(-100000);
}

void MFStepper::setMaxSpeed(uint16_t speed)
{
    if (!_initialized)
        return;

    _stepper->setMaxSpeed(speed);
}

void MFStepper::setAcceleration(uint16_t acceleration)
{
    if (!_initialized)
        return;

    _stepper->setAcceleration(acceleration);
}

void MFStepper::powerSavingMode(bool state)
{
    if (!_initialized)
        return;
    if (state)
        _stepper->disableOutputs();
    else
        _stepper->enableOutputs();
}

// MFStepper.cpp
