#include "mobiflight.h"
#include "CustomDevice.h"
#include "MFCustomDevice.h"

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
        about custom input devices.
    ********************************************************************************** */
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
/*
    void readConfigFromFlash()
    {
        if (configLength == 0) // do nothing if no config is available
            return;
        uint16_t addreeprom   = MEM_OFFSET_CONFIG;               // define first memory location where config is saved in EEPROM
        uint16_t addrbuffer   = 0;                               // and start with first memory location from nameBuffer
        char     params[8]    = "";                              // buffer for reading parameters from EEPROM and sending to ::Add() function of device
        uint8_t  command      = readUintFromEEPROM(&addreeprom); // read the first value from EEPROM, it's a device definition
        bool     copy_success = true;                            // will be set to false if copying input names to nameBuffer exceeds array dimensions
                                                                // not required anymore when pins instead of names are transferred to the UI

        if (command == 0) // just to be sure, configLength should also be 0
            return;

        getArraysizes();

        do // go through the EEPROM until it is NULL terminated
        {
            switch (command) {
            case kTypeButton:
                params[0] = readUintFromEEPROM(&addreeprom);                             // Pin number
                Button::Add(params[0], &nameBuffer[addrbuffer]);                         // MUST be before readNameFromEEPROM because readNameFromEEPROM returns the pointer for the NEXT Name
                copy_success = readNameFromEEPROM(&addreeprom, nameBuffer, &addrbuffer); // copy the NULL terminated name to nameBuffer and set to next free memory location
                break;

            case kTypeOutput:
                params[0] = readUintFromEEPROM(&addreeprom); // Pin number
                Output::Add(params[0]);
                copy_success = readEndCommandFromEEPROM(&addreeprom, ':'); // check EEPROM until end of name
                break;

    #if MF_SEGMENT_SUPPORT == 1
            // this is for backwards compatibility
            case kTypeLedSegmentDeprecated:
            // this is the new type
            case kTypeLedSegmentMulti:
                params[0] = LedSegment::TYPE_MAX72XX;
                if (command == kTypeLedSegmentMulti)
                    params[0] = readUintFromEEPROM(&addreeprom); // Type of LedSegment

                params[1] = readUintFromEEPROM(&addreeprom); // Pin Data number
                params[2] = readUintFromEEPROM(&addreeprom); // Pin CS number
                params[3] = readUintFromEEPROM(&addreeprom); // Pin CLK number
                params[4] = readUintFromEEPROM(&addreeprom); // brightness
                params[5] = readUintFromEEPROM(&addreeprom); // number of modules
                LedSegment::Add(params[0], params[1], params[2], params[3], params[5], params[4]);
                copy_success = readEndCommandFromEEPROM(&addreeprom, ':'); // check EEPROM until end of name
                break;
    #endif

    #if MF_STEPPER_SUPPORT == 1
            case kTypeStepperDeprecated1:
            case kTypeStepperDeprecated2:
            case kTypeStepper:
                // Values for all stepper types
                params[0] = readUintFromEEPROM(&addreeprom); // Pin1 number
                params[1] = readUintFromEEPROM(&addreeprom); // Pin2 number
                params[2] = readUintFromEEPROM(&addreeprom); // Pin3 number
                params[3] = readUintFromEEPROM(&addreeprom); // Pin4 number

                // Default values for older types
                params[4] = (uint8_t)0; // Button number
                params[5] = (uint8_t)0; // Stepper Mode
                params[6] = (uint8_t)0; // backlash
                params[7] = false;      // deactivate output

                if (command == kTypeStepperDeprecated2 || command == kTypeStepper) {
                    params[4] = readUintFromEEPROM(&addreeprom); // Button number
                }

                if (command == kTypeStepper) {
                    params[5] = readUintFromEEPROM(&addreeprom); // Stepper Mode
                    params[6] = readUintFromEEPROM(&addreeprom); // backlash
                    params[7] = readUintFromEEPROM(&addreeprom); // deactivate output
                }
                // there is an additional 9th parameter stored in the config (profileID) which is not needed in the firmware
                // and therefor not read in, it is just skipped like the name with reading until end of command
                Stepper::Add(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7]);
                copy_success = readEndCommandFromEEPROM(&addreeprom, ':'); // check EEPROM until end of name
                break;
    #endif

    #if MF_SERVO_SUPPORT == 1
            case kTypeServo:
                params[0] = readUintFromEEPROM(&addreeprom); // Pin number
                Servos::Add(params[0]);
                copy_success = readEndCommandFromEEPROM(&addreeprom, ':'); // check EEPROM until end of name
                break;
    #endif

            case kTypeEncoderSingleDetent:
            case kTypeEncoder:
                params[0] = readUintFromEEPROM(&addreeprom); // Pin1 number
                params[1] = readUintFromEEPROM(&addreeprom); // Pin2 number
                params[2] = 0;                               // type

                if (command == kTypeEncoder)
                    params[2] = readUintFromEEPROM(&addreeprom); // type

                Encoder::Add(params[0], params[1], params[2], &nameBuffer[addrbuffer]);  // MUST be before readNameFromEEPROM because readNameFromEEPROM returns the pointer for the NEXT Name
                copy_success = readNameFromEEPROM(&addreeprom, nameBuffer, &addrbuffer); // copy the NULL terminated name to nameBuffer and set to next free memory location
                break;

    #if MF_LCD_SUPPORT == 1
            case kTypeLcdDisplayI2C:
                params[0] = readUintFromEEPROM(&addreeprom); // address
                params[1] = readUintFromEEPROM(&addreeprom); // columns
                params[2] = readUintFromEEPROM(&addreeprom); // lines
                LCDDisplay::Add(params[0], params[1], params[2]);
                copy_success = readEndCommandFromEEPROM(&addreeprom, ':'); // check EEPROM until end of name
                break;
    #endif

    #if MF_ANALOG_SUPPORT == 1
            case kTypeAnalogInput:
                params[0] = readUintFromEEPROM(&addreeprom);                             // pin number
                params[1] = readUintFromEEPROM(&addreeprom);                             // sensitivity
                Analog::Add(params[0], &nameBuffer[addrbuffer], params[1]);              // MUST be before readNameFromEEPROM because readNameFromEEPROM returns the pointer for the NEXT Name
                copy_success = readNameFromEEPROM(&addreeprom, nameBuffer, &addrbuffer); // copy the NULL terminated name to to nameBuffer and set to next free memory location
                                                                                        //    copy_success = readEndCommandFromEEPROM(&addreeprom, ':');       // once the nameBuffer is not required anymore uncomment this line and delete the line before
                break;
    #endif

    #if MF_OUTPUT_SHIFTER_SUPPORT == 1
            case kTypeOutputShifter:
                params[0] = readUintFromEEPROM(&addreeprom); // latch Pin
                params[1] = readUintFromEEPROM(&addreeprom); // clock Pin
                params[2] = readUintFromEEPROM(&addreeprom); // data Pin
                params[3] = readUintFromEEPROM(&addreeprom); // number of daisy chained modules
                OutputShifter::Add(params[0], params[1], params[2], params[3]);
                copy_success = readEndCommandFromEEPROM(&addreeprom, ':'); // check EEPROM until end of name
                break;
    #endif

    #if MF_INPUT_SHIFTER_SUPPORT == 1
            case kTypeInputShifter:
                params[0] = readUintFromEEPROM(&addreeprom); // latch Pin
                params[1] = readUintFromEEPROM(&addreeprom); // clock Pin
                params[2] = readUintFromEEPROM(&addreeprom); // data Pin
                params[3] = readUintFromEEPROM(&addreeprom); // number of daisy chained modules
                InputShifter::Add(params[0], params[1], params[2], params[3], &nameBuffer[addrbuffer]);
                copy_success = readNameFromEEPROM(&addreeprom, nameBuffer, &addrbuffer); // copy the NULL terminated name to to nameBuffer and set to next free memory location
                                                                                        //    copy_success = readEndCommandFromEEPROM(&addreeprom, ':');       // once the nameBuffer is not required anymore uncomment this line and delete the line before
                break;
    #endif

    #if MF_MUX_SUPPORT == 1
                // No longer a separate config command for the mux driver
                // case kTypeMuxDriver:
                //   params[0] = strtok_r(NULL, ".", &p); // Sel0 pin
                //   params[1] = strtok_r(NULL, ".", &p); // Sel1 pin
                //   params[2] = strtok_r(NULL, ".", &p); // Sel2 pin
                //   params[3] = strtok_r(NULL, ":", &p); // Sel3 pin
                //   AddMultiplexer(atoi(params[0]), atoi(params[1]), atoi(params[2]), atoi(params[3]));
                //   break;
    #endif

    #if MF_DIGIN_MUX_SUPPORT == 1
            case kTypeDigInMux:
                params[0] = readUintFromEEPROM(&addreeprom); // data pin
                // Mux driver section
                // Repeated commands do not define more objects, but change the only existing one
                // therefore beware that all DigInMux configuration commands are consistent!
                params[1] = readUintFromEEPROM(&addreeprom); // Sel0 pin
                params[2] = readUintFromEEPROM(&addreeprom); // Sel1 pin
                params[3] = readUintFromEEPROM(&addreeprom); // Sel2 pin
                params[4] = readUintFromEEPROM(&addreeprom); // Sel3 pin
                MUX.attach(params[1], params[2], params[3], params[4]);
                params[5] = readUintFromEEPROM(&addreeprom); // 8-bit registers (1-2)
                DigInMux::Add(params[0], params[5], &nameBuffer[addrbuffer]);
                copy_success = readNameFromEEPROM(&addreeprom, nameBuffer, &addrbuffer);

                // cmdMessenger.sendCmd(kDebug, F("Mux loaded"));
                break;
    #endif

    #if MF_CUSTOMDEVICE_SUPPORT == 1
            case kTypeCustomDevice: {
                uint16_t adrType = addreeprom; // first location of custom Type in EEPROM
                copy_success     = readEndCommandFromEEPROM(&addreeprom, '.');
                if (!copy_success)
                    break;

                uint16_t adrPin = addreeprom; // first location of custom pins in EEPROM
                copy_success    = readEndCommandFromEEPROM(&addreeprom, '.');
                if (!copy_success)
                    break;

                uint16_t adrConfig = addreeprom; // first location of custom config in EEPROM
                copy_success       = readEndCommandFromEEPROM(&addreeprom, '.');
                if (copy_success) {
                    CustomDevice::Add(adrPin, adrType, adrConfig);
                    copy_success = readEndCommandFromEEPROM(&addreeprom, ':'); // check EEPROM until end of command
                }
                // cmdMessenger.sendCmd(kDebug, F("CustomDevice loaded"));
                break;
            }
    #endif

            default:
                copy_success = readEndCommandFromEEPROM(&addreeprom, ':'); // check EEPROM until end of name
            }
            command = readUintFromEEPROM(&addreeprom);
        } while (command && copy_success);
        if (!copy_success) {                            // too much/long names for input devices
            nameBuffer[MEMLEN_NAMES_BUFFER - 1] = 0x00; // terminate the last copied (part of) string with 0x00
            cmdMessenger.sendCmd(kStatus, F("Failure on reading config"));
        }
    }
*/
} // end of namespace
