//
// mobiflight.cpp
//
// (C) MobiFlight Project 2022
//

#include <Arduino.h>
#include "config.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "Button.h"
#include "Encoder.h"
#include "MFEEPROM.h"
#ifdef MF_ANALOG_SUPPORT
#include "Analog.h"
#endif
#ifdef MF_INPUT_SHIFTER_SUPPORT
#include "InputShifter.h"
#endif
#include "Output.h"
#ifdef MF_SEGMENT_SUPPORT
#include "LedSegment.h"
#endif
#ifdef MF_STEPPER_SUPPORT
#include "Stepper.h"
#endif
#ifdef MF_SERVO_SUPPORT
#include "Servos.h"
#endif
#ifdef MF_LCD_SUPPORT
#include "LCDDisplay.h"
#endif
#ifdef MF_OUTPUT_SHIFTER_SUPPORT
#include "OutputShifter.h"
#endif
#ifdef MF_DIGIN_MUX_SUPPORT
#include "DigInMux.h"
#include "MFDigInMux.h"
#endif
#ifdef MF_CUSTOMDEVICE_SUPPORT
#include "CustomDevice.h"
#endif

#define MF_BUTTON_DEBOUNCE_MS     10 // time between updating the buttons
#define MF_ENCODER_DEBOUNCE_MS    1  // time between encoder updates
#define MF_INSHIFTER_POLL_MS      10 // time between input shift reg updates
#define MF_INMUX_POLL_MS          10 // time between dig input mux updates
#define MF_SERVO_DELAY_MS         5  // time between servo updates
#define MF_ANALOGAVERAGE_DELAY_MS 10 // time between updating the analog average calculation
#define MF_ANALOGREAD_DELAY_MS    50 // time between sending analog values

bool                powerSavingMode   = false;
const unsigned long POWER_SAVING_TIME = 60 * 15; // in seconds

#if MF_MUX_SUPPORT == 1
MFMuxDriver MUX;
#endif
// ==================================================
//   Polling interval counters
// ==================================================

typedef struct {
    uint32_t Buttons  = 0;
    uint32_t Encoders = 0;
#ifdef MF_SERVO_SUPPORT
    uint32_t Servos = 0;
#endif
#ifdef MF_ANALOG_SUPPORT
    uint32_t AnalogAverage = 0;
    uint32_t Analog        = 0;
#endif
#ifdef MF_INPUT_SHIFTER_SUPPORT
    uint32_t InputShifters = 0;
#endif
#ifdef MF_DIGIN_MUX_SUPPORT
    uint32_t DigInMux = 0;
#endif
#ifdef MF_CUSTOMDEVICE_SUPPORT
    uint32_t CustomDevice = 0;
#endif
} lastUpdate_t;

lastUpdate_t lastUpdate;

extern MFEEPROM MFeeprom;

void initPollIntervals(void)
{
    // Init Time Gap between Inputs, do not read at the same loop
    lastUpdate.Buttons  = millis();
    lastUpdate.Encoders = millis();
#ifdef MF_SERVO_SUPPORT
    lastUpdate.Servos = millis() + 2;
#endif
#ifdef MF_ANALOG_SUPPORT
    lastUpdate.AnalogAverage = millis() + 4;
    lastUpdate.Analog        = millis() + 4;
#endif
#ifdef MF_INPUT_SHIFTER_SUPPORT
    lastUpdate.InputShifters = millis() + 6;
#endif
#ifdef MF_DIGIN_MUX_SUPPORT
    lastUpdate.DigInMux = millis() + 8;
#endif
}

void timedUpdate(void (*fun)(), uint32_t *last, uint8_t intv)
{
    if (millis() - *last >= intv) {
        fun();
        *last = millis();
    }
}

// ************************************************************
// Power saving
// ************************************************************
void SetPowerSavingMode(bool state)
{
    // disable the lights ;)
    powerSavingMode = state;
    Output::PowerSave(state);
#ifdef MF_SEGMENT_SUPPORT
    LedSegment::PowerSave(state);
#endif
#ifdef MF_STEPPER_SUPPORT
    Stepper::PowerSave(state);
#endif
#ifdef MF_CUSTOMDEVICE_SUPPORT
    CustomDevice::PowerSave(state);
#endif
#ifdef MF_OUTPUT_SHIFTER_SUPPORT
    OutputShifter::PowerSave(state);
#endif
#ifdef MF_LCD_SUPPORT
    LCDDisplay::PowerSave(state);
#endif

#ifdef DEBUG2CMDMESSENGER
    if (state)
        cmdMessenger.sendCmd(kDebug, F("Power saving mode on"));
    else
        cmdMessenger.sendCmd(kDebug, F("Power saving mode off"));
#endif
}

void updatePowerSaving()
{
    if (!powerSavingMode && ((millis() - getLastCommandMillis()) > (POWER_SAVING_TIME * 1000))) {
        // enable power saving
        SetPowerSavingMode(true);
    } else if (powerSavingMode && ((millis() - getLastCommandMillis()) < (POWER_SAVING_TIME * 1000))) {
        // disable power saving
        SetPowerSavingMode(false);
    }
}

// ************************************************************
// Reset Board
// ************************************************************
void ResetBoard()
{
    restoreName();
    loadConfig();
}

// ************************************************************
// Setup
// ************************************************************
void setup()
{
    Serial.begin(115200);
    MFeeprom.init();
    attachCommandCallbacks();
    cmdMessenger.printLfCr();
    ResetBoard();
    initPollIntervals();
}

// ************************************************************
// Loop function
// ************************************************************
void loop()
{
    // Process incoming serial data, and perform callbacks
    cmdMessenger.feedinSerialData();
    updatePowerSaving();

    // if config has been reset and still is not activated
    // do not perform updates
    // to prevent mangling input for config (shared buffers)
    if (getStatusConfig()) {

        timedUpdate(Button::read, &lastUpdate.Buttons, MF_BUTTON_DEBOUNCE_MS);

        timedUpdate(Encoder::read, &lastUpdate.Encoders, MF_ENCODER_DEBOUNCE_MS);

#ifdef MF_STEPPER_SUPPORT
        Stepper::update();
#endif

#ifdef MF_SERVO_SUPPORT
        timedUpdate(Servos::update, &lastUpdate.Servos, MF_SERVO_DELAY_MS);
#endif

#ifdef MF_ANALOG_SUPPORT
        timedUpdate(Analog::read, &lastUpdate.Analog, MF_ANALOGREAD_DELAY_MS);
        timedUpdate(Analog::readAverage, &lastUpdate.AnalogAverage, MF_ANALOGAVERAGE_DELAY_MS);
#endif

#ifdef MF_INPUT_SHIFTER_SUPPORT
        timedUpdate(InputShifter::read, &lastUpdate.InputShifters, MF_INSHIFTER_POLL_MS);
#endif

#ifdef MF_DIGIN_MUX_SUPPORT
        timedUpdate(DigInMux::read, &lastUpdate.DigInMux, MF_INMUX_POLL_MS);
#endif

#if defined(MF_CUSTOMDEVICE_SUPPORT) && defined(MF_CUSTOMDEVICE_HAS_UPDATE)
#ifdef MF_CUSTOMDEVICE_POLL_MS
        timedUpdate(CustomDevice::update, &lastUpdate.CustomDevice, MF_CUSTOMDEVICE_POLL_MS);
#else
        CustomDevice::update();
#endif
#endif

        // lcds, outputs, outputshifters, segments do not need update
    }
}

// mobiflight.cpp
