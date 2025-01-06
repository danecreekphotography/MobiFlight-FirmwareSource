//
// commandmessenger.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <CmdMessenger.h>

// This is the list of recognized commands. These can be commands that can either be sent or received.
// In order to receive, attach a callback function to these events

enum {
    kInitModule,                   // 0
    kSetModule,                    // 1
    kSetPin,                       // 2
    kSetStepper,                   // 3
    kSetServo,                     // 4
    kStatus,                       // 5, Command to report status
    kEncoderChangeDeprecated,      // 6
    kButtonChangeDeprecated,       // 7
    kStepperChange,                // 8
    kGetInfo,                      // 9
    kInfo,                         // 10
    kSetConfig,                    // 11
    kGetConfig,                    // 12
    kResetConfig,                  // 13
    kSaveConfig,                   // 14
    kConfigSaved,                  // 15
    kActivateConfig,               // 16
    kConfigActivated,              // 17
    kSetPowerSavingMode,           // 18
    kSetName,                      // 19
    kGenNewSerial,                 // 20
    kResetStepper,                 // 21
    kSetZeroStepper,               // 22
    kTrigger,                      // 23
    kResetBoard,                   // 24
    kSetLcdDisplayI2C,             // 25
    kSetModuleBrightness,          // 26
    kSetShiftRegisterPins,         // 27
    kAnalogChangeDeprecated,       // 28
    kInputShifterChangeDeprecated, // 29
    kDigInMuxChangeDeprecated,     // 30
    kSetStepperSpeedAccel,         // 31
    kSetCustomDevice,              // 32
    kSetModuleSingleSegment,       // 33
    // functions which get be called from the UI via the command messenger
    // must be added before. The command messenger has a fix buffer size
    // for the functions which defined by MAXCALLBACKS=30.
    // -------------------------------------------------------------------
    // new handler ID's for input devices must be added here
    // This will save size of the Callback buffer
    kButtonChange = 100, // 100
    kEncoderChange,      // 101
    kAnalogChange,       // 102
    kInputShifterChange, // 103
    kDigInMuxChange,     // 104
    kDebug = 0xFF        // 255
};

void     attachCommandCallbacks();
uint32_t getLastCommandMillis();
void     setLastCommandMillis();

extern CmdMessenger cmdMessenger;