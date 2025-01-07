//
// config.h
//
// (C) MobiFlight Project 2022
//

#pragma once

enum {
    kTypeNotSet,               // 0
    kTypeButton,               // 1
    kTypeEncoderSingleDetent,  // 2 (retained for backwards compatibility, use kTypeEncoder for new configs)
    kTypeOutput,               // 3
    kTypeLedSegmentDeprecated, // 4 (keep for backwards compatibility)
    kTypeStepperDeprecated1,   // 5 (keep for backwards compatibility, doesn't support autohome)
    kTypeServo,                // 6
    kTypeLcdDisplayI2C,        // 7
    kTypeEncoder,              // 8
    kTypeStepperDeprecated2,   // 9 (keep for backwards compatibility, stepper type with auto zero support if btnPin is > 0)
    kTypeOutputShifter,        // 10 Shift register support (example: 74HC595, TLC592X)
    kTypeAnalogInputDeprecated,// 11 Analog Device with 1 pin
    kTypeInputShifter,         // 12 Input shift register support (example: 74HC165)
    kTypeMuxDriver,            // 13 Multiplexer selector support (generates select outputs)
    kTypeDigInMux,             // 14 Digital input multiplexer support (example: 74HCT4067, 74HCT4051)
    kTypeStepper,              // 15 new stepper type with settings for backlash and deactivate output
    kTypeLedSegmentMulti,      // 16 new led segment with MAX7219 and TM1637 support
    kTypeCustomDevice,         // 17 Custom Device
    kTypeAnalogInput,          // 18 Analog Device without PullUp enabled
    kTypeMax                   // if new device types are added, this MUST be before this one!
};

void loadConfig(void);
bool getStatusConfig(void);
void generateSerial(bool);
void OnSetConfig(void);
void OnResetConfig(void);
void OnSaveConfig(void);
void OnActivateConfig(void);
void OnGetConfig(void);
void OnGetInfo(void);
void OnGenNewSerial(void);
void OnSetName(void);
void restoreName(void);
bool getBoardReady();

// config.h