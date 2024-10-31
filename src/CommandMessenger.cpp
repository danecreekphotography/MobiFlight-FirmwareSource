//
// commandmessenger.cpp
//
// (C) MobiFlight Project 2022
//

#include "commandmessenger.h"
#include "config.h"
#include "Button.h"
#include "Encoder.h"
#if defined(MF_ANALOG_SUPPORT)
#include "Analog.h"
#endif
#if defined(MF_INPUT_SHIFTER_SUPPORT)
#include "InputShifter.h"
#endif
#include "Output.h"
#if defined(MF_SEGMENT_SUPPORT)
#include "LedSegment.h"
#endif
#if defined(MF_STEPPER_SUPPORT)
#include "Stepper.h"
#endif
#if defined(MF_SERVO_SUPPORT)
#include "Servos.h"
#endif
#if defined(MF_LCD_SUPPORT)
#include "LCDDisplay.h"
#endif
#if defined(MF_OUTPUT_SHIFTER_SUPPORT)
#include "OutputShifter.h"
#endif
#if defined(MF_DIGIN_MUX_SUPPORT)
#include "DigInMux.h"
#endif
#if defined(MF_CUSTOMDEVICE_SUPPORT)
#include "CustomDevice.h"
#endif

CmdMessenger  cmdMessenger = CmdMessenger(Serial);
unsigned long lastCommand;

void OnSetPowerSavingMode();
void OnTrigger();
void OnUnknownCommand();

// Callbacks define on which received commands we take action
void attachCommandCallbacks()
{
    // Attach callback methods
    cmdMessenger.attach(OnUnknownCommand);

#if defined(MF_SEGMENT_SUPPORT)
    cmdMessenger.attach(kInitModule, LedSegment::OnInitModule);
    cmdMessenger.attach(kSetModule, LedSegment::OnSetModule);
    cmdMessenger.attach(kSetModuleBrightness, LedSegment::OnSetModuleBrightness);
    cmdMessenger.attach(kSetModuleSingleSegment, LedSegment::OnSetModuleSingleSegment);
#endif

    cmdMessenger.attach(kSetPin, Output::OnSet);

#if defined(MF_STEPPER_SUPPORT)
    cmdMessenger.attach(kSetStepper, Stepper::OnSet);
    cmdMessenger.attach(kResetStepper, Stepper::OnReset);
    cmdMessenger.attach(kSetZeroStepper, Stepper::OnSetZero);
    cmdMessenger.attach(kSetStepperSpeedAccel, Stepper::OnSetSpeedAccel);
#endif

#if defined(MF_SERVO_SUPPORT)
    cmdMessenger.attach(kSetServo, Servos::OnSet);
#endif

    cmdMessenger.attach(kGetInfo, OnGetInfo);
    cmdMessenger.attach(kGetConfig, OnGetConfig);
    cmdMessenger.attach(kSetConfig, OnSetConfig);
    cmdMessenger.attach(kResetConfig, OnResetConfig);
    cmdMessenger.attach(kSaveConfig, OnSaveConfig);
    cmdMessenger.attach(kActivateConfig, OnActivateConfig);
    cmdMessenger.attach(kSetName, OnSetName);
    cmdMessenger.attach(kGenNewSerial, OnGenNewSerial);
    cmdMessenger.attach(kTrigger, OnTrigger);
    cmdMessenger.attach(kSetPowerSavingMode, OnSetPowerSavingMode);

#if defined(MF_LCD_SUPPORT)
    cmdMessenger.attach(kSetLcdDisplayI2C, LCDDisplay::OnSet);
#endif

#if defined(MF_OUTPUT_SHIFTER_SUPPORT)
    cmdMessenger.attach(kSetShiftRegisterPins, OutputShifter::OnSet);
#endif

#if defined(MF_CUSTOMDEVICE_SUPPORT)
    cmdMessenger.attach(kSetCustomDevice, CustomDevice::OnSet);
#endif

#if defined(DEBUG2CMDMESSENGER)
    cmdMessenger.sendCmd(kDebug, F("Attached callbacks"));
#endif
}

// Called when a received command has no attached function
void OnUnknownCommand()
{
    lastCommand = millis();
    cmdMessenger.sendCmd(kStatus, F("n/a"));
}

// Handles requests from the desktop app to disable power saving mode
void OnSetPowerSavingMode()
{
    bool enablePowerSavingMode = cmdMessenger.readBoolArg();

    // If the request is to enable powersaving mode then set the last command time
    // to the earliest possible time. The next time loop() is called in mobiflight.cpp
    // this will cause power saving mode to get turned on.
    if (enablePowerSavingMode) {
#if defined(DEBUG2CMDMESSENGER)
        cmdMessenger.sendCmd(kDebug, F("Enabling power saving mode via request"));
#endif
        lastCommand = 0;
    }
    // If the request is to disable power saving mode then simply set the last command
    // to now. The next time loop() is called in mobiflight.cpp this will cause
    // power saving mode to get turned off.
    else {
#if defined(DEBUG2CMDMESSENGER)
        cmdMessenger.sendCmd(kDebug, F("Disabling power saving mode via request"));
#endif
        lastCommand = millis();
    }
}

uint32_t getLastCommandMillis()
{
    return lastCommand;
}

void OnTrigger()
{
    Button::OnTrigger();
#if defined(MF_INPUT_SHIFTER_SUPPORT)
    InputShifter::OnTrigger();
#endif
#if defined(MF_DIGIN_MUX_SUPPORT)
    DigInMux::OnTrigger();
#endif
#if defined(MF_ANALOG_SUPPORT)
    Analog::OnTrigger();
#endif
}

// commandmessenger.cpp
