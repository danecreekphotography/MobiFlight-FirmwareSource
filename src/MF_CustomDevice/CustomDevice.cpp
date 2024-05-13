#include "mobiflight.h"
#include "CustomDevice.h"
#include "MFCustomDevice.h"
#include "Button.h"
#include "Encoder.h"
#include "Output.h"
#if MF_ANALOG_SUPPORT == 1
#include "Analog.h"
#endif
#if MF_INPUT_SHIFTER_SUPPORT == 1
#include "InputShifter.h"
#endif
#if MF_SEGMENT_SUPPORT == 1
#include "LedSegment.h"
#endif
#if MF_STEPPER_SUPPORT == 1
#include "Stepper.h"
#endif
#if MF_SERVO_SUPPORT == 1
#include "Servos.h"
#endif
#if MF_LCD_SUPPORT == 1
#include "LCDDisplay.h"
#endif
#if MF_OUTPUT_SHIFTER_SUPPORT == 1
#include "OutputShifter.h"
#endif
#if MF_MUX_SUPPORT == 1
#include "MFMuxDriver.h"
#endif
#if MF_DIGIN_MUX_SUPPORT == 1
#include "DigInMux.h"
#endif
#if MF_CUSTOMDEVICE_SUPPORT == 1
#include "CustomDevice.h"
#endif

extern char nameBuffer[];
#if MF_MUX_SUPPORT == 1
extern MFMuxDriver MUX;
#endif

/* **********************************************************************************
    Normally nothing has to be changed in this file
    It handles one or multiple custom devices
********************************************************************************** */

#define MESSAGEID_POWERSAVINGMODE -2

namespace CustomDevice
{
    MFCustomDevice *customDevice;
    uint8_t         customDeviceRegistered = 0;
    uint8_t         maxCustomDevices       = 0;

    bool setupArray(uint16_t count)
    {
        if (!FitInMemory(sizeof(MFCustomDevice) * count))
            return false;
        customDevice     = new (allocateMemory(sizeof(MFCustomDevice) * count)) MFCustomDevice();
        maxCustomDevices = count;
        return true;
    }

    void Add(uint16_t adrPin, uint16_t adrType, uint16_t adrConfig)
    {
        if (customDeviceRegistered == maxCustomDevices)
            return;
        customDevice[customDeviceRegistered] = MFCustomDevice();
        customDevice[customDeviceRegistered].attach(adrPin, adrType, adrConfig);
        customDeviceRegistered++;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kStatus, F("Added CustomDevice"));
#endif
    }

    /* **********************************************************************************
        All custom devives gets unregistered if a new config gets uploaded.
        The Clear() function from every registerd custom device will be called.
        Keep it as it is, mostly nothing must be changed
    ********************************************************************************** */
    void Clear()
    {
        for (int i = 0; i != customDeviceRegistered; i++) {
            customDevice[i].detach();
        }
        customDeviceRegistered = 0;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kStatus, F("Cleared CustomDevice"));
#endif
    }

    /* **********************************************************************************
        Within in loop() the update() function is called regularly
        Within the loop() you can define a time delay where this function gets called
        or as fast as possible. See comments in loop()
        The update() function from every registerd custom device will be called.
        It is only needed if you have to update your custom device without getting
        new values from the connector. Otherwise just make a return;
    ********************************************************************************** */
    void update()
    {
        for (int i = 0; i != customDeviceRegistered; i++) {
            customDevice[i].update();
        }
    }

    /* **********************************************************************************
        If an output for the custom device is defined in the connector,
        this function gets called when a new value is available.
        In this case the connector sends a messageID followed by a string which is not longer
        than 90 Byte (!!check the length of the string!!) limited by the commandMessenger.
        The messageID is used to mark the value which has changed. This reduced the serial
        communication as not all values has to be send in one (big) string (like for the LCD)
        The OnSet() function from every registerd custom device will be called.
    ********************************************************************************** */
    void OnSet()
    {
        int16_t device = cmdMessenger.readInt16Arg(); // get the device number
        if (device >= customDeviceRegistered)         // and do nothing if this device is not registered
            return;
        int16_t messageID = cmdMessenger.readInt16Arg();  // get the messageID number
        char   *output    = cmdMessenger.readStringArg(); // get the pointer to the new raw string
        cmdMessenger.unescape(output);                    // and unescape the string if escape characters are used
        customDevice[device].set(messageID, output);      // send the string to your custom device
    }

    /* **********************************************************************************
        This function is called if the status of the PowerSavingMode changes.
        'state' is true if PowerSaving is enabled
        'state' is false if PowerSaving is disabled
        MessageID '-2' for the custom device  for PowerSavingMode
    ********************************************************************************** */
    void PowerSave(bool state)
    {
        for (uint8_t i = 0; i < customDeviceRegistered; ++i) {
            if (state)
                customDevice[i].set(MESSAGEID_POWERSAVINGMODE, "1");
            else
                customDevice[i].set(MESSAGEID_POWERSAVINGMODE, "0");
        }
    }

    /* **********************************************************************************
        These functions are called after startup to inform the connector
        about the config stored in the Flash and to load the config from Flash
    ********************************************************************************** */
    bool CheckConfigFlash()
    {
        if (MFCustomDeviceGetConfig() == NULL)
            return false;
        return true;
    }

    bool GetConfigFromFlash()
    {
        uint16_t* CustomDeviceConfig = MFCustomDeviceGetConfig();
        uint8_t readBytefromFlash = pgm_read_byte_near(CustomDeviceConfig++);

        if (readBytefromFlash == 0)
            return false;
        cmdMessenger.sendCmdArg((char)readBytefromFlash);
        readBytefromFlash = pgm_read_byte_near(CustomDeviceConfig++);
        do {
            cmdMessenger.sendArg((char)readBytefromFlash);
            readBytefromFlash = pgm_read_byte_near(CustomDeviceConfig++);
        } while (readBytefromFlash != 0);

        return true;
    }

    // reads an ascii value which is '.' terminated from Flash and returns it's value
    uint8_t readUintFromFlash(uint16_t *addrFlash)
    {
        char    params[4] = {0}; // max 3 (255) digits NULL terminated
        uint8_t counter   = 0;
        do {
            params[counter++] = (char)pgm_read_byte_near(addrFlash++);
            if (params[counter - 1] == 0)
                return 0;
        } while (params[counter - 1] != '.' && counter < sizeof(params));
        params[counter - 1] = 0x00;
        return atoi(params);
    }

    void GetArraySizesFromFlash(uint8_t numberDevices[])
    {
        uint16_t* addrFlash = MFCustomDeviceGetConfig();
        uint8_t  device    = readUintFromFlash(addrFlash);

        if (device == 0)
            return;

        do {
            numberDevices[device]++;
            while (pgm_read_byte_near(addrFlash) != ':') {
                addrFlash++;
            }
            device = readUintFromFlash(++addrFlash);
        } while (device);
    }

    // reads a string from Flash at given address which is ':' terminated and saves it in the nameBuffer
    // once the nameBuffer is not needed anymore, just read until the ":" termination -> see function below
    bool readNameFromFlash(uint16_t *addrFlash, char *buffer, uint16_t *addrbuffer)
    {
        char temp = 0;
        do {
            temp                    = pgm_read_byte_near((addrFlash)++); // read the first character
            buffer[(*addrbuffer)++] = temp;                                // save character and locate next buffer position
            if (*addrbuffer >= MEMLEN_NAMES_BUFFER) {                      // nameBuffer will be exceeded
                return false;                                              // abort copying from EEPROM to nameBuffer
            }
        } while (temp != ':');            // reads until limiter ':' and locates the next free buffer position
        buffer[(*addrbuffer) - 1] = 0x00; // replace ':' by NULL, terminates the string
        return true;
    }

    // steps thru the Flash until the delimiter is detected
    // it could be ":" for end of one device config
    // or "." for end of type/pin/config entry for custom device
    bool readEndCommandFromFlash(uint16_t *addrFlash, uint8_t delimiter)
    {
        char     temp   = 0;
        do {
            temp = pgm_read_byte_near((addrFlash)++);
        } while (temp != delimiter); // reads until limiter ':'
        return true;
    }

    void ReadConfigFromFlash()
    {
        uint16_t *addrFlash   = MFCustomDeviceGetConfig();
        uint16_t addrbuffer   = 0;                               // and start with first memory location from nameBuffer
        char     params[8]    = "";                              // buffer for reading parameters from EEPROM and sending to ::Add() function of device
        uint8_t  command      = readUintFromFlash(addrFlash); // read the first value from EEPROM, it's a device definition
        bool     copy_success = true;                            // will be set to false if copying input names to nameBuffer exceeds array dimensions
                                                                // not required anymore when pins instead of names are transferred to the UI

        if (pgm_read_byte_near(addrFlash) == 0x00)
            return;

        // getArraysizes(); is done within reading from eeprom, but think about the oder (eeprom / flash)

        do // go through the Flash until it is NULL terminated
        {
            switch (command) {
            case kTypeButton:
                params[0] = readUintFromFlash(addrFlash);                             // Pin number
                Button::Add(params[0], &nameBuffer[addrbuffer]);                         // MUST be before readNameFromFlash because readNameFromFlash returns the pointer for the NEXT Name
                copy_success = readNameFromFlash(addrFlash, nameBuffer, &addrbuffer); // copy the NULL terminated name to nameBuffer and set to next free memory location
                break;

            case kTypeOutput:
                params[0] = readUintFromFlash(addrFlash); // Pin number
                Output::Add(params[0]);
                copy_success = readEndCommandFromFlash(addrFlash, ':'); // check EEPROM until end of name
                break;

    #if MF_SEGMENT_SUPPORT == 1
            // this is for backwards compatibility
            case kTypeLedSegmentDeprecated:
            // this is the new type
            case kTypeLedSegmentMulti:
                params[0] = LedSegment::TYPE_MAX72XX;
                if (command == kTypeLedSegmentMulti)
                    params[0] = readUintFromFlash(addrFlash); // Type of LedSegment

                params[1] = readUintFromFlash(addrFlash); // Pin Data number
                params[2] = readUintFromFlash(addrFlash); // Pin CS number
                params[3] = readUintFromFlash(addrFlash); // Pin CLK number
                params[4] = readUintFromFlash(addrFlash); // brightness
                params[5] = readUintFromFlash(addrFlash); // number of modules
                LedSegment::Add(params[0], params[1], params[2], params[3], params[5], params[4]);
                copy_success = readEndCommandFromFlash(addrFlash, ':'); // check EEPROM until end of name
                break;
    #endif

    #if MF_STEPPER_SUPPORT == 1
            case kTypeStepperDeprecated1:
            case kTypeStepperDeprecated2:
            case kTypeStepper:
                // Values for all stepper types
                params[0] = readUintFromFlash(addrFlash); // Pin1 number
                params[1] = readUintFromFlash(addrFlash); // Pin2 number
                params[2] = readUintFromFlash(addrFlash); // Pin3 number
                params[3] = readUintFromFlash(addrFlash); // Pin4 number

                // Default values for older types
                params[4] = (uint8_t)0; // Button number
                params[5] = (uint8_t)0; // Stepper Mode
                params[6] = (uint8_t)0; // backlash
                params[7] = false;      // deactivate output

                if (command == kTypeStepperDeprecated2 || command == kTypeStepper) {
                    params[4] = readUintFromFlash(addrFlash); // Button number
                }

                if (command == kTypeStepper) {
                    params[5] = readUintFromFlash(addrFlash); // Stepper Mode
                    params[6] = readUintFromFlash(addrFlash); // backlash
                    params[7] = readUintFromFlash(addrFlash); // deactivate output
                }
                // there is an additional 9th parameter stored in the config (profileID) which is not needed in the firmware
                // and therefor not read in, it is just skipped like the name with reading until end of command
                Stepper::Add(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7]);
                copy_success = readEndCommandFromFlash(addrFlash, ':'); // check EEPROM until end of name
                break;
    #endif

    #if MF_SERVO_SUPPORT == 1
            case kTypeServo:
                params[0] = readUintFromFlash(addrFlash); // Pin number
                Servos::Add(params[0]);
                copy_success = readEndCommandFromFlash(addrFlash, ':'); // check EEPROM until end of name
                break;
    #endif

            case kTypeEncoderSingleDetent:
            case kTypeEncoder:
                params[0] = readUintFromFlash(addrFlash); // Pin1 number
                params[1] = readUintFromFlash(addrFlash); // Pin2 number
                params[2] = 0;                               // type

                if (command == kTypeEncoder)
                    params[2] = readUintFromFlash(addrFlash); // type

                Encoder::Add(params[0], params[1], params[2], &nameBuffer[addrbuffer]);  // MUST be before readNameFromFlash because readNameFromFlash returns the pointer for the NEXT Name
                copy_success = readNameFromFlash(addrFlash, nameBuffer, &addrbuffer); // copy the NULL terminated name to nameBuffer and set to next free memory location
                break;

    #if MF_LCD_SUPPORT == 1
            case kTypeLcdDisplayI2C:
                params[0] = readUintFromFlash(addrFlash); // address
                params[1] = readUintFromFlash(addrFlash); // columns
                params[2] = readUintFromFlash(addrFlash); // lines
                LCDDisplay::Add(params[0], params[1], params[2]);
                copy_success = readEndCommandFromFlash(addrFlash, ':'); // check EEPROM until end of name
                break;
    #endif

    #if MF_ANALOG_SUPPORT == 1
            case kTypeAnalogInput:
                params[0] = readUintFromFlash(addrFlash);                             // pin number
                params[1] = readUintFromFlash(addrFlash);                             // sensitivity
                Analog::Add(params[0], &nameBuffer[addrbuffer], params[1]);              // MUST be before readNameFromFlash because readNameFromFlash returns the pointer for the NEXT Name
                copy_success = readNameFromFlash(addrFlash, nameBuffer, &addrbuffer); // copy the NULL terminated name to to nameBuffer and set to next free memory location
                                                                                        //    copy_success = readEndCommandFromFlash(addrFlash, ':');       // once the nameBuffer is not required anymore uncomment this line and delete the line before
                break;
    #endif

    #if MF_OUTPUT_SHIFTER_SUPPORT == 1
            case kTypeOutputShifter:
                params[0] = readUintFromFlash(addrFlash); // latch Pin
                params[1] = readUintFromFlash(addrFlash); // clock Pin
                params[2] = readUintFromFlash(addrFlash); // data Pin
                params[3] = readUintFromFlash(addrFlash); // number of daisy chained modules
                OutputShifter::Add(params[0], params[1], params[2], params[3]);
                copy_success = readEndCommandFromFlash(addrFlash, ':'); // check EEPROM until end of name
                break;
    #endif

    #if MF_INPUT_SHIFTER_SUPPORT == 1
            case kTypeInputShifter:
                params[0] = readUintFromFlash(addrFlash); // latch Pin
                params[1] = readUintFromFlash(addrFlash); // clock Pin
                params[2] = readUintFromFlash(addrFlash); // data Pin
                params[3] = readUintFromFlash(addrFlash); // number of daisy chained modules
                InputShifter::Add(params[0], params[1], params[2], params[3], &nameBuffer[addrbuffer]);
                copy_success = readNameFromFlash(addrFlash, nameBuffer, &addrbuffer); // copy the NULL terminated name to to nameBuffer and set to next free memory location
                                                                                        //    copy_success = readEndCommandFromFlash(addrFlash, ':');       // once the nameBuffer is not required anymore uncomment this line and delete the line before
                break;
    #endif

    #if MF_DIGIN_MUX_SUPPORT == 1
            case kTypeDigInMux:
                params[0] = readUintFromFlash(addrFlash); // data pin
                // Mux driver section
                // Repeated commands do not define more objects, but change the only existing one
                // therefore beware that all DigInMux configuration commands are consistent!
                params[1] = readUintFromFlash(addrFlash); // Sel0 pin
                params[2] = readUintFromFlash(addrFlash); // Sel1 pin
                params[3] = readUintFromFlash(addrFlash); // Sel2 pin
                params[4] = readUintFromFlash(addrFlash); // Sel3 pin
                MUX.attach(params[1], params[2], params[3], params[4]);
                params[5] = readUintFromFlash(addrFlash); // 8-bit registers (1-2)
                DigInMux::Add(params[0], params[5], &nameBuffer[addrbuffer]);
                copy_success = readNameFromFlash(addrFlash, nameBuffer, &addrbuffer);

                // cmdMessenger.sendCmd(kDebug, F("Mux loaded"));
                break;
    #endif
/*
    #if MF_CUSTOMDEVICE_SUPPORT == 1
            case kTypeCustomDevice: {
                uint16_t adrType = addrFlash; // first location of custom Type in EEPROM
                copy_success     = readEndCommandFromFlash(addrFlash, '.');
                if (!copy_success)
                    break;

                uint16_t adrPin = addrFlash; // first location of custom pins in EEPROM
                copy_success    = readEndCommandFromFlash(addrFlash, '.');
                if (!copy_success)
                    break;

                uint16_t adrConfig = addrFlash; // first location of custom config in EEPROM
                copy_success       = readEndCommandFromFlash(addrFlash, '.');
                if (copy_success) {
                    CustomDevice::Add(adrPin, adrType, adrConfig);
                    copy_success = readEndCommandFromFlash(addrFlash, ':'); // check EEPROM until end of command
                }
                // cmdMessenger.sendCmd(kDebug, F("CustomDevice loaded"));
                break;
            }
    #endif
*/
            default:
                copy_success = readEndCommandFromFlash(addrFlash, ':'); // check EEPROM until end of name
            }
            command = readUintFromFlash(addrFlash);
        } while (command && copy_success);
        if (!copy_success) {                            // too much/long names for input devices
            nameBuffer[MEMLEN_NAMES_BUFFER - 1] = 0x00; // terminate the last copied (part of) string with 0x00
            cmdMessenger.sendCmd(kStatus, F("Failure on reading config"));
        }
    }

} // end of namespace
