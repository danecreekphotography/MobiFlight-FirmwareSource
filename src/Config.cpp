//
// config.cpp
//
// (C) MobiFlight Project 2022
//

#include "mobiflight.h"
#include "MFEEPROM.h"
#include "Button.h"
#include "Encoder.h"
#include "Output.h"
#if defined(ARDUINO_ARCH_RP2040)
#include "ArduinoUniqueID.h"
#endif

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
#ifdef HAS_CONFIG_IN_FLASH
#include "MFCustomDevicesConfig.h"
#else
const char CustomDeviceConfig[] PROGMEM = {};
#endif

// The build version comes from an environment variable
#define STRINGIZER(arg) #arg
#define STR_VALUE(arg)  STRINGIZER(arg)
#define VERSION         STR_VALUE(BUILD_VERSION)
#define CORE_VERSION    STR_VALUE(CORE_BUILD_VERSION)

MFEEPROM MFeeprom;

#if MF_MUX_SUPPORT == 1
extern MFMuxDriver MUX;
#endif

const uint8_t MEM_OFFSET_NAME   = 0;
const uint8_t MEM_LEN_NAME      = 48;
const uint8_t MEM_OFFSET_SERIAL = MEM_OFFSET_NAME + MEM_LEN_NAME;
const uint8_t MEM_LEN_SERIAL    = 11;
const uint8_t MEM_OFFSET_CONFIG = MEM_OFFSET_NAME + MEM_LEN_NAME + MEM_LEN_SERIAL;

#if defined(ARDUINO_ARCH_AVR)
char serial[11] = MOBIFLIGHT_SERIAL; // 3 characters for "SN-",7 characters for "xyz-zyx" plus terminating NULL
#elif defined(ARDUINO_ARCH_RP2040)
char serial[3 + UniqueIDsize * 2 + 1] = MOBIFLIGHT_SERIAL; // 3 characters for "SN-", UniqueID as HEX String, terminating NULL
#endif
char      name[MEM_LEN_NAME]              = MOBIFLIGHT_NAME;
const int MEM_LEN_CONFIG                  = MEMLEN_CONFIG;
char      nameBuffer[MEMLEN_NAMES_BUFFER] = "";
uint16_t  configLengthEEPROM              = 0;
boolean   configActivated                 = false;
uint16_t  pNameBuffer                     = 0; // pointer for nameBuffer during reading of config
#if MF_CUSTOMDEVICE_SUPPORT == 1
const uint8_t configLengthFlash = sizeof(CustomDeviceConfig);
#endif

void resetConfig();
void readConfig();
void _activateConfig();
void readConfigFromMemory(bool configFromFlash);

// ************************************************************
// configBuffer handling
// ************************************************************
// reads the EEPROM until NUL terminator and returns the number of characters incl. terminator, starting from given address
bool readconfigLengthEEPROM()
{
    uint16_t addreeprom = MEM_OFFSET_CONFIG;
    uint16_t length     = MFeeprom.get_length();
    configLengthEEPROM  = 0;

    if (MFeeprom.read_byte(MEM_OFFSET_CONFIG) == 0xFF)
        return false;
    while (MFeeprom.read_byte(addreeprom++) != 0x00) {
        configLengthEEPROM++;
        if (addreeprom > length) {
            cmdMessenger.sendCmd(kStatus, F("Loading config failed")); // text or "-1" like config upload?
            return false;
        }
    }
    return true;
}

void loadConfig()
{
#ifdef DEBUG2CMDMESSENGER
    cmdMessenger.sendCmd(kDebug, F("Load config"));
#endif
    readconfigLengthEEPROM();
    readConfig();
    _activateConfig();
}

void OnSetConfig()
{
#ifdef DEBUG2CMDMESSENGER
    cmdMessenger.sendCmd(kDebug, F("Setting config start"));
#endif
#if MF_CUSTOMDEVICE_SUPPORT == 1
    // For now a config can only be saved if NO config in flash is available
    // otherwise a config from flash would be sent from the connector
    // and saved in the EEPROM, so the config get's doubled
    if (configLengthFlash == 0)
#endif
    {
        char   *cfg    = cmdMessenger.readStringArg();
        uint8_t cfgLen = strlen(cfg);

        if (configLengthEEPROM + cfgLen + 1 < MEM_LEN_CONFIG) {
            MFeeprom.write_block(MEM_OFFSET_CONFIG + configLengthEEPROM, cfg, cfgLen + 1); // save the received config string including the terminatung NULL (+1) to EEPROM
            configLengthEEPROM += cfgLen;
            cmdMessenger.sendCmd(kStatus, configLengthEEPROM);
        } else
            cmdMessenger.sendCmd(kStatus, -1); // last successfull saving block is already NULL terminated, nothing more todo
    }
#ifdef DEBUG2CMDMESSENGER
    cmdMessenger.sendCmd(kDebug, F("Setting config end"));
#endif
}

void resetConfig()
{
    Button::Clear();
    Encoder::Clear();
    Output::Clear();
#if MF_SEGMENT_SUPPORT == 1
    LedSegment::Clear();
#endif
#if MF_SERVO_SUPPORT == 1
    Servos::Clear();
#endif
#if MF_STEPPER_SUPPORT == 1
    Stepper::Clear();
#endif
#if MF_LCD_SUPPORT == 1
    LCDDisplay::Clear();
#endif
#if MF_ANALOG_SUPPORT == 1
    Analog::Clear();
#endif
#if MF_OUTPUT_SHIFTER_SUPPORT == 1
    OutputShifter::Clear();
#endif
#if MF_INPUT_SHIFTER_SUPPORT == 1
    InputShifter::Clear();
#endif
#if MF_DIGIN_MUX_SUPPORT == 1
    DigInMux::Clear();
#endif
#if MF_CUSTOMDEVICE_SUPPORT == 1
    CustomDevice::Clear();
#endif
    configLengthEEPROM = 0;
    configActivated    = false;
    pNameBuffer        = 0;
    ClearMemory();
}

void OnResetConfig()
{
    resetConfig();
    cmdMessenger.sendCmd(kStatus, F("OK"));
}

void OnSaveConfig()
{
    cmdMessenger.sendCmd(kConfigSaved, F("OK"));
}

void OnActivateConfig()
{
    readConfig();
    _activateConfig();
}

void _activateConfig()
{
    configActivated = true;
    cmdMessenger.sendCmd(kConfigActivated, F("OK"));
}

// reads an ascii value which is '.' terminated from EEPROM or Flash and returns it's value
uint8_t readUint(volatile uint16_t *addrMem, bool configFromFlash)
{
    char    params[4] = {0}; // max 3 (255) digits NULL terminated
    uint8_t counter   = 0;
    do {
#if MF_CUSTOMDEVICE_SUPPORT == 1
        if (configFromFlash) {
            params[counter++] = pgm_read_byte_near(CustomDeviceConfig + (*addrMem)++);
        } else
#endif
        {
            params[counter++] = MFeeprom.read_byte((*addrMem)++);
        }
    } while (params[counter - 1] != '.' && counter < sizeof(params)); // reads until limiter '.' and for safety reason not more then size of params[]
    params[counter - 1] = 0x00; // replace '.' by NULL to terminate the string
    return atoi(params);
}

// reads a string from EEPROM or Flash at given address which is ':' terminated and saves it in the nameBuffer
// once the nameBuffer is not needed anymore, just read until the ":" termination -> see function below
bool readName(uint16_t *addrMem, char *buffer, uint16_t *pBuffer, bool configFromFlash)
{
    char     temp   = 0;
    uint16_t length = MFeeprom.get_length();
    do {
#if MF_CUSTOMDEVICE_SUPPORT == 1
        if (configFromFlash) {
            temp = pgm_read_byte_near(CustomDeviceConfig + (*addrMem)++); // read the first character
            if (*addrMem > configLengthFlash)                             // abort if config array size will be exceeded
                return false;
        } else
#endif
        {
            temp = MFeeprom.read_byte((*addrMem)++); // read the first character
            if (*addrMem > length)                   // abort if EEPROM size will be exceeded
                return false;
        }
        buffer[(*pBuffer)++] = temp;         // save character and locate next buffer position
        if (*pBuffer >= MEMLEN_NAMES_BUFFER) // nameBuffer will be exceeded
        {
            return false; // abort copying from EEPROM to nameBuffer
        }
    } while (temp != ':'); // reads until limiter ':' and locates the next free buffer position
    buffer[(*pBuffer) - 1] = 0x00; // replace ':' by NULL, terminates the string
    return true;
}

// steps thru the EEPRROM or Flash until the delimiter is detected
// it could be ":" for end of one device config
// or "." for end of type/pin/config entry for custom device
bool readEndCommand(uint16_t *addrMem, uint8_t delimiter, bool configFromFlash)
{
    char     temp   = 0;
    uint16_t length = MFeeprom.get_length();
    do {
#if MF_CUSTOMDEVICE_SUPPORT == 1
        if (configFromFlash) {
            temp = pgm_read_byte_near(CustomDeviceConfig + (*addrMem)++);
            if (*addrMem > configLengthFlash) // abort if config array size will be exceeded
                return false;
        } else
#endif
        {
            temp = MFeeprom.read_byte((*addrMem)++);
            if (*addrMem > length) // abort if EEPROM size will be exceeded
                return false;
        }
    } while (temp != delimiter);
    return true;
}

void sendFailureMessage(const char *deviceName)
{
    cmdMessenger.sendCmdStart(kStatus);
    cmdMessenger.sendCmdArg(deviceName);
    cmdMessenger.sendCmdArg(F("does not fit in Memory"));
    cmdMessenger.sendCmdEnd();
}

bool GetArraySizes(uint8_t *numberDevices, bool configFromFlash)
{
    bool     copy_success = true;
    uint16_t addrMem;
    uint8_t  device;
    if (configFromFlash)
        addrMem = 0;
    else
        addrMem = MEM_OFFSET_CONFIG;

    device = readUint(&addrMem, configFromFlash);

    // step through the Memory and calculate the number of devices for each type
    do {
        numberDevices[device]++;
        copy_success = readEndCommand(&addrMem, ':', configFromFlash); // check EEPROM until end of name
        device       = readUint(&addrMem, configFromFlash);
    } while (device && copy_success);

    if (!copy_success) { // too much/long names for input devices -> tbd how to handle this!!
        cmdMessenger.sendCmd(kStatus, F("Failure, EEPROM size exceeded "));
        return false;
    }
    return true;
}

void InitArrays(uint8_t *numberDevices)
{
    // Call the function to allocate required memory for the arrays of each type
    if (!Button::setupArray(numberDevices[kTypeButton]))
        sendFailureMessage("Button");
    if (!Output::setupArray(numberDevices[kTypeOutput]))
        sendFailureMessage("Output");
#if MF_SEGMENT_SUPPORT == 1
    if (!LedSegment::setupArray(numberDevices[kTypeLedSegmentDeprecated] + numberDevices[kTypeLedSegmentMulti]))
        sendFailureMessage("7Segment");
#endif
#if MF_STEPPER_SUPPORT == 1
    if (!Stepper::setupArray(numberDevices[kTypeStepper] + numberDevices[kTypeStepperDeprecated1] + numberDevices[kTypeStepperDeprecated2]))
        sendFailureMessage("Stepper");
#endif
#if MF_SERVO_SUPPORT == 1
    if (!Servos::setupArray(numberDevices[kTypeServo]))
        sendFailureMessage("Servo");
#endif
    if (!Encoder::setupArray(numberDevices[kTypeEncoder] + numberDevices[kTypeEncoderSingleDetent]))
        sendFailureMessage("Encoders");
#if MF_LCD_SUPPORT == 1
    if (!LCDDisplay::setupArray(numberDevices[kTypeLcdDisplayI2C]))
        sendFailureMessage("LCD");
#endif
#if MF_ANALOG_SUPPORT == 1
    if (!Analog::setupArray(numberDevices[kTypeAnalogInput]))
        sendFailureMessage("AnalogIn");
#endif
#if MF_OUTPUT_SHIFTER_SUPPORT == 1
    if (!OutputShifter::setupArray(numberDevices[kTypeOutputShifter]))
        sendFailureMessage("OutputShifter");
#endif
#if MF_INPUT_SHIFTER_SUPPORT == 1
    if (!InputShifter::setupArray(numberDevices[kTypeInputShifter]))
        sendFailureMessage("InputShifter");
#endif
#if MF_DIGIN_MUX_SUPPORT == 1
    if (!DigInMux::setupArray(numberDevices[kTypeDigInMux]))
        sendFailureMessage("DigInMux");
#endif
#if MF_CUSTOMDEVICE_SUPPORT == 1
    if (!CustomDevice::setupArray(numberDevices[kTypeCustomDevice]))
        sendFailureMessage("CustomDevice");
#endif
    return;
}

void readConfig()
{
    uint8_t numberDevices[kTypeMax] = {0};
#if MF_CUSTOMDEVICE_SUPPORT == 1
    if (configLengthFlash > 0) {
        GetArraySizes(numberDevices, true);
    }
#endif
    if (configLengthEEPROM > 0) {
        GetArraySizes(numberDevices, false);
    }
    InitArrays(numberDevices);
#if MF_CUSTOMDEVICE_SUPPORT == 1
    if (configLengthFlash > 0) {
        readConfigFromMemory(true);
    }
#endif
    if (configLengthEEPROM > 0) {
        readConfigFromMemory(false);
    }
}

void readConfigFromMemory(bool configFromFlash)
{
    uint16_t addrMem      = 0;    // define first memory location where config is saved in EEPROM or Flash
    char     params[8]    = "";   // buffer for reading parameters from EEPROM or Flash and sending to ::Add() function of device
    uint8_t  command      = 0;    // read the first value from EEPROM or Flash, it's a device definition
    bool     copy_success = true; // will be set to false if copying input names to nameBuffer exceeds array dimensions
                                  // not required anymore when pins instead of names are transferred to the UI

    if (!configFromFlash) {
        addrMem = MEM_OFFSET_CONFIG;
    }

    // read the first value from EEPROM, it's a device definition
    command = readUint(&addrMem, configFromFlash);

    // go through the EEPROM or Flash until it is NULL terminated
    do {
        switch (command) {
        case kTypeButton:
            params[0] = readUint(&addrMem, configFromFlash);                              // Pin number
            Button::Add(params[0], &nameBuffer[pNameBuffer]);                             // MUST be before readName because readName returns the pointer for the NEXT Name
            copy_success = readName(&addrMem, nameBuffer, &pNameBuffer, configFromFlash); // copy the NULL terminated name to nameBuffer and set to next free memory location
            break;

        case kTypeOutput:
            params[0] = readUint(&addrMem, configFromFlash); // Pin number
            Output::Add(params[0]);
            copy_success = readEndCommand(&addrMem, ':', configFromFlash); // check EEPROM until end of name
            break;

#if MF_SEGMENT_SUPPORT == 1
        // this is for backwards compatibility
        case kTypeLedSegmentDeprecated:
        // this is the new type
        case kTypeLedSegmentMulti:
            params[0] = LedSegment::TYPE_MAX72XX;
            if (command == kTypeLedSegmentMulti)
                params[0] = readUint(&addrMem, configFromFlash); // Type of LedSegment

            params[1] = readUint(&addrMem, configFromFlash); // Pin Data number
            params[2] = readUint(&addrMem, configFromFlash); // Pin CS number
            params[3] = readUint(&addrMem, configFromFlash); // Pin CLK number
            params[4] = readUint(&addrMem, configFromFlash); // brightness
            params[5] = readUint(&addrMem, configFromFlash); // number of modules
            LedSegment::Add(params[0], params[1], params[2], params[3], params[5], params[4]);
            copy_success = readEndCommand(&addrMem, ':', configFromFlash); // check EEPROM until end of name
            break;
#endif

#if MF_STEPPER_SUPPORT == 1
        case kTypeStepperDeprecated1:
        case kTypeStepperDeprecated2:
        case kTypeStepper:
            // Values for all stepper types
            params[0] = readUint(&addrMem, configFromFlash); // Pin1 number
            params[1] = readUint(&addrMem, configFromFlash); // Pin2 number
            params[2] = readUint(&addrMem, configFromFlash); // Pin3 number
            params[3] = readUint(&addrMem, configFromFlash); // Pin4 number

            // Default values for older types
            params[4] = (uint8_t)0; // Button number
            params[5] = (uint8_t)0; // Stepper Mode
            params[6] = (uint8_t)0; // backlash
            params[7] = false;      // deactivate output

            if (command == kTypeStepperDeprecated2 || command == kTypeStepper) {
                params[4] = readUint(&addrMem, configFromFlash); // Button number
            }

            if (command == kTypeStepper) {
                params[5] = readUint(&addrMem, configFromFlash); // Stepper Mode
                params[6] = readUint(&addrMem, configFromFlash); // backlash
                params[7] = readUint(&addrMem, configFromFlash); // deactivate output
            }
            // there is an additional 9th parameter stored in the config (profileID) which is not needed in the firmware
            // and therefor not read in, it is just skipped like the name with reading until end of command
            Stepper::Add(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7]);
            copy_success = readEndCommand(&addrMem, ':', configFromFlash); // check EEPROM until end of name
            break;
#endif

#if MF_SERVO_SUPPORT == 1
        case kTypeServo:
            params[0] = readUint(&addrMem, configFromFlash); // Pin number
            Servos::Add(params[0]);
            copy_success = readEndCommand(&addrMem, ':', configFromFlash); // check EEPROM until end of name
            break;
#endif

        case kTypeEncoderSingleDetent:
        case kTypeEncoder:
            params[0] = readUint(&addrMem, configFromFlash); // Pin1 number
            params[1] = readUint(&addrMem, configFromFlash); // Pin2 number
            params[2] = 0;                                   // type

            if (command == kTypeEncoder)
                params[2] = readUint(&addrMem, configFromFlash); // type

            Encoder::Add(params[0], params[1], params[2], &nameBuffer[pNameBuffer]);      // MUST be before readName because readName returns the pointer for the NEXT Name
            copy_success = readName(&addrMem, nameBuffer, &pNameBuffer, configFromFlash); // copy the NULL terminated name to nameBuffer and set to next free memory location
            break;

#if MF_LCD_SUPPORT == 1
        case kTypeLcdDisplayI2C:
            params[0] = readUint(&addrMem, configFromFlash); // address
            params[1] = readUint(&addrMem, configFromFlash); // columns
            params[2] = readUint(&addrMem, configFromFlash); // lines
            LCDDisplay::Add(params[0], params[1], params[2]);
            copy_success = readEndCommand(&addrMem, ':', configFromFlash); // check EEPROM until end of name
            break;
#endif

#if MF_ANALOG_SUPPORT == 1
        case kTypeAnalogInput:
            params[0] = readUint(&addrMem, configFromFlash);                              // pin number
            params[1] = readUint(&addrMem, configFromFlash);                              // sensitivity
            Analog::Add(params[0], &nameBuffer[pNameBuffer], params[1]);                  // MUST be before readName because readName returns the pointer for the NEXT Name
            copy_success = readName(&addrMem, nameBuffer, &pNameBuffer, configFromFlash); // copy the NULL terminated name to to nameBuffer and set to next free memory location
                                                                                          //    copy_success = readEndCommand(&addrMem, ':');       // once the nameBuffer is not required anymore uncomment this line and delete the line before
            break;
#endif

#if MF_OUTPUT_SHIFTER_SUPPORT == 1
        case kTypeOutputShifter:
            params[0] = readUint(&addrMem, configFromFlash); // latch Pin
            params[1] = readUint(&addrMem, configFromFlash); // clock Pin
            params[2] = readUint(&addrMem, configFromFlash); // data Pin
            params[3] = readUint(&addrMem, configFromFlash); // number of daisy chained modules
            OutputShifter::Add(params[0], params[1], params[2], params[3]);
            copy_success = readEndCommand(&addrMem, ':', configFromFlash); // check EEPROM until end of name
            break;
#endif

#if MF_INPUT_SHIFTER_SUPPORT == 1
        case kTypeInputShifter:
            params[0] = readUint(&addrMem, configFromFlash); // latch Pin
            params[1] = readUint(&addrMem, configFromFlash); // clock Pin
            params[2] = readUint(&addrMem, configFromFlash); // data Pin
            params[3] = readUint(&addrMem, configFromFlash); // number of daisy chained modules
            InputShifter::Add(params[0], params[1], params[2], params[3], &nameBuffer[pNameBuffer]);
            copy_success = readName(&addrMem, nameBuffer, &pNameBuffer, configFromFlash); // copy the NULL terminated name to to nameBuffer and set to next free memory location
                                                                                          //    copy_success = readEndCommand(&addrMem, ':');       // once the nameBuffer is not required anymore uncomment this line and delete the line before
            break;
#endif

#if MF_DIGIN_MUX_SUPPORT == 1
        case kTypeDigInMux:
            params[0] = readUint(&addrMem, configFromFlash); // data pin
            // Mux driver section
            // Repeated commands do not define more objects, but change the only existing one
            // therefore beware that all DigInMux configuration commands are consistent!
            params[1] = readUint(&addrMem, configFromFlash); // Sel0 pin
            params[2] = readUint(&addrMem, configFromFlash); // Sel1 pin
            params[3] = readUint(&addrMem, configFromFlash); // Sel2 pin
            params[4] = readUint(&addrMem, configFromFlash); // Sel3 pin
            MUX.attach(params[1], params[2], params[3], params[4]);
            params[5] = readUint(&addrMem, configFromFlash); // 8-bit registers (1-2)
            DigInMux::Add(params[0], params[5], &nameBuffer[pNameBuffer]);
            copy_success = readName(&addrMem, nameBuffer, &pNameBuffer, configFromFlash);
            break;
#endif

#if MF_CUSTOMDEVICE_SUPPORT == 1
        case kTypeCustomDevice: {
            uint16_t adrType = addrMem; // first location of custom Type in EEPROM
            copy_success     = readEndCommand(&addrMem, '.', configFromFlash);
            if (!copy_success)
                break;

            uint16_t adrPin = addrMem; // first location of custom pins in EEPROM
            copy_success    = readEndCommand(&addrMem, '.', configFromFlash);
            if (!copy_success)
                break;

            uint16_t adrConfig = addrMem; // first location of custom config in EEPROM
            copy_success       = readEndCommand(&addrMem, '.', configFromFlash);
            if (copy_success) {
                CustomDevice::Add(adrPin, adrType, adrConfig, configFromFlash);
                copy_success = readEndCommand(&addrMem, ':', configFromFlash); // check EEPROM until end of command
            }
            break;
        }
#endif

        default:
            copy_success = readEndCommand(&addrMem, ':', configFromFlash); // check EEPROM until end of name
        }
        command = readUint(&addrMem, configFromFlash);
    } while (command && copy_success);
    if (!copy_success) {                            // too much/long names for input devices
        nameBuffer[MEMLEN_NAMES_BUFFER - 1] = 0x00; // terminate the last copied (part of) string with 0x00
        cmdMessenger.sendCmd(kStatus, F("Failure on reading config"));
    }
}

void OnGetConfig()
{
    bool sentFromFlash = false;
    cmdMessenger.sendCmdStart(kInfo);
#if MF_CUSTOMDEVICE_SUPPORT == 1
    if (configLengthFlash > 0) {
        cmdMessenger.sendCmdArg((char)pgm_read_byte_near(CustomDeviceConfig));
        for (uint16_t i = 1; i < configLengthFlash - 1; i++) {
            cmdMessenger.sendArg((char)pgm_read_byte_near(CustomDeviceConfig + i));
        }
        sentFromFlash = true;
    }
#endif
    if (configLengthEEPROM > 0) {
        if (sentFromFlash)
            cmdMessenger.sendArg((char)MFeeprom.read_byte(MEM_OFFSET_CONFIG));
        else
            cmdMessenger.sendCmdArg((char)MFeeprom.read_byte(MEM_OFFSET_CONFIG));
        for (uint16_t i = 1; i < configLengthEEPROM; i++) {
            cmdMessenger.sendArg((char)MFeeprom.read_byte(MEM_OFFSET_CONFIG + i));
        }
    }
    cmdMessenger.sendCmdEnd();
}

void OnGetInfo()
{
    // read the serial number and generate if 1st start up, was before in ResetBoard()
    // moved to this position as the time to generate a serial number in ResetBoard() is always the same
    // OnGetInfo() is called from the connector and the time is very likely always different
    // Therefore millis() can be used for randomSeed
    generateSerial(false);
    cmdMessenger.sendCmdStart(kInfo);
    cmdMessenger.sendCmdArg(F(MOBIFLIGHT_TYPE));
    cmdMessenger.sendCmdArg(name);
    cmdMessenger.sendCmdArg(serial);
    cmdMessenger.sendCmdArg(VERSION);
    cmdMessenger.sendCmdArg(CORE_VERSION);
    cmdMessenger.sendCmdEnd();
}

bool getStatusConfig()
{
    return configActivated;
}

// ************************************************************
// serial number handling
// ************************************************************
void generateRandomSerial()
{
    randomSeed(millis());
    serial[0]             = 'S';
    serial[1]             = 'N';
    serial[2]             = '-';
    serial[6]             = '-';
    serial[10]            = 0x00;
    uint16_t randomSerial = random(4095);
    for (uint8_t i = 3; i < 6; i++) {
        serial[i] = (randomSerial & 0x000F) + 48; // convert from 4bit to HEX string
        if (serial[i] >= 58) serial[i] += 8;      // if HeX value is A - F add 8 to get the letters
        randomSerial >>= 4;
    }
    randomSerial = random(4095);
    for (uint8_t i = 7; i < 10; i++) {
        serial[i] = (randomSerial & 0x000F) + 48; // convert from 4bit to HEX string
        if (serial[i] >= 58) serial[i] += 7;      // if HeX value is A - F add 7 to get the letters
        randomSerial >>= 4;
    }
    MFeeprom.write_block(MEM_OFFSET_SERIAL, serial, MEM_LEN_SERIAL);
    cmdMessenger.sendCmd(kDebug, F("Serial number generated"));
}

#if defined(ARDUINO_ARCH_RP2040)
void readUniqueSerial()
{
    serial[0] = 'S';
    serial[1] = 'N';
    serial[2] = '-';
    for (size_t i = 0; i < UniqueIDsize; i++) {
        serial[3 + i * 2] = (UniqueID[i] >> 4) + 48;
        if (serial[3 + i * 2] >= 58) serial[3 + i * 2] += 7;
        serial[3 + i * 2 + 1] = (UniqueID[i] & 0x0F) + 48;
        if (serial[3 + i * 2 + 1] >= 58) serial[3 + i * 2 + 1] += 7;
    }
}
#endif

void generateSerial(bool force)
{
    if (force) {
        // A serial number is forced to generate
        // generate a serial number acc. the old style also for the Pico
        generateRandomSerial();
        return;
    }

    // A serial number according old style is already generated and saved to the eeprom
    if (MFeeprom.read_byte(MEM_OFFSET_SERIAL) == 'S' && MFeeprom.read_byte(MEM_OFFSET_SERIAL + 1) == 'N') {
        MFeeprom.read_block(MEM_OFFSET_SERIAL, serial, MEM_LEN_SERIAL);
        return;
    }

    // A uniqueID is already generated and saved to the eeprom
    if (MFeeprom.read_byte(MEM_OFFSET_SERIAL) == 'I' && MFeeprom.read_byte(MEM_OFFSET_SERIAL + 1) == 'D') {
#if defined(ARDUINO_ARCH_AVR)
        generateRandomSerial();
#elif defined(ARDUINO_ARCH_RP2040)
        readUniqueSerial();
#endif
        return;
    }

    // Coming here no UniqueID and no serial number is available, so it's the first start up of a board
#if defined(ARDUINO_ARCH_AVR)
    // Generate a serial number for AVR's
    // To have not always the same starting point for the random generator, millis() are
    // used as starting point. It is very unlikely that the time between flashing the firmware
    // and getting the command to send the info's to the connector is always the same.
    // additional double check if it's really a new board, should reduce Jaimes problem
    generateRandomSerial();
#elif defined(ARDUINO_ARCH_RP2040)
    // Read the uniqueID for Pico's and use it as serial number
    readUniqueSerial();
    // mark this in the eeprom that a UniqueID is used on first start up for Pico's
    MFeeprom.write_block(MEM_OFFSET_SERIAL, "ID", 2);
#endif
    if (MFeeprom.read_byte(MEM_OFFSET_CONFIG) == 0xFF) {
        MFeeprom.write_block(MEM_OFFSET_CONFIG, 0x00);
    }
}

void OnGenNewSerial()
{
    generateSerial(true);
    cmdMessenger.sendCmd(kInfo, serial);
}

// ************************************************************
// Naming handling
// ************************************************************
void storeName()
{
    MFeeprom.write_byte(MEM_OFFSET_NAME, '#');
    MFeeprom.write_block(MEM_OFFSET_NAME + 1, name, MEM_LEN_NAME - 1);
}

void restoreName()
{
    if (MFeeprom.read_byte(MEM_OFFSET_NAME) != '#')
        return;

    MFeeprom.read_block(MEM_OFFSET_NAME + 1, name, MEM_LEN_NAME - 1);
}

void OnSetName()
{
    char *cfg = cmdMessenger.readStringArg();
    memcpy(name, cfg, MEM_LEN_NAME);
    storeName();
    cmdMessenger.sendCmd(kStatus, name);
}

// config.cpp
