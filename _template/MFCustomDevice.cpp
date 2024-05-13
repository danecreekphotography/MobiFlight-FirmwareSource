#include "MFCustomDevice.h"
#include "commandmessenger.h"
#include "allocateMem.h"
#include "MFEEPROM.h"
#include "MFCustomDevicesConfig.h"
extern MFEEPROM MFeeprom;

/* **********************************************************************************
    This function is called after startup to inform the connector
    about config in Flash
    The devices are defined in MFCustomDevicesConfig.h
********************************************************************************** */
char* MFCustomDeviceGetConfig() {
    return (char*)CustomDeviceConfig;
}

bool MFCustomDeviceConfigFlash()
{
    if (pgm_read_byte_near(MFCustomDeviceGetConfig()) == 0x00) {
        return false;
    }
    return true;
}

/* **********************************************************************************
    The custom device pins, type and configuration is stored in the EEPROM
    While loading the config the adresses in the EEPROM are transferred to the constructor
    Within the constructor you have to copy the EEPROM content to a buffer
    and evaluate him. The buffer is used for all 3 types (pins, type configuration),
    so do it step by step.
    The max size of the buffer is defined here. It must be the size of the
    expected max length of these strings.

    E.g. 6 pins are required, each pin could have two characters (two digits),
    each pins are delimited by "|" and the string is NULL terminated.
    -> (6 * 2) + 5 + 1 = 18 bytes is the maximum.
    The custom type is "MyCustomClass", which means 14 characters plus NULL = 15
    The configuration is "myConfig", which means 8 characters plus NULL = 9
    The maximum characters to be expected is 18, so MEMLEN_STRING_BUFFER has to be at least 18
********************************************************************************** */
#define MEMLEN_STRING_BUFFER 40

// reads a string from EEPROM at given address which is '.' terminated and saves it to the buffer
bool MFCustomDevice::getStringFromEEPROM(uint16_t addreeprom, char *buffer)
{
    char    temp    = 0;
    uint8_t counter = 0;
    do {
        temp              = MFeeprom.read_byte((addreeprom)++); // read the first character
        buffer[counter++] = temp;                               // save character and locate next buffer position
        if (counter >= MEMLEN_STRING_BUFFER) {                  // nameBuffer will be exceeded
            return false;                                       // abort copying to buffer
        }
    } while (temp != '.');      // reads until limiter '.' and locates the next free buffer position
    buffer[counter - 1] = 0x00; // replace '.' by NULL, terminates the string
    return true;
}

MFCustomDevice::MFCustomDevice()
{
    _initialized = false;
}

/* **********************************************************************************
    Within the connector pins, a device name and a config string can be defined
    These informations are stored in the EEPROM like for the other devices.
    While reading the config from the EEPROM this function is called.
    It is the first function which will be called for the custom device.
    If it fits into the memory buffer, the constructor for the customer device
    will be called
********************************************************************************** */

void MFCustomDevice::attach(uint16_t adrPin, uint16_t adrType, uint16_t adrConfig)
{
    if (adrPin == 0) return;

    /* **********************************************************************************
        Do something which is required to setup your custom device
    ********************************************************************************** */

    char   *params, *p = NULL;
    char    parameter[MEMLEN_STRING_BUFFER];
    uint8_t _pin1, _pin2, _pin3;

    /* **********************************************************************************
        Read the Type from the EEPROM, copy it into a buffer and evaluate it
        The string get's NOT stored as this would need a lot of RAM, instead a variable
        is used to store the type
    ********************************************************************************** */
    getStringFromEEPROM(adrType, parameter);
    if (strcmp(parameter, "MOBIFLIGHT_TEMPLATE") == 0)
        _customType = MY_CUSTOM_DEVICE_1;
    if (strcmp(parameter, "MOBIFLIGHT_TEMPLATE2") == 0)
        _customType = MY_CUSTOM_DEVICE_2;

    if (_customType == MY_CUSTOM_DEVICE_1) {
        /* **********************************************************************************
            Check if the device fits into the device buffer
        ********************************************************************************** */
        if (!FitInMemory(sizeof(MyCustomClass))) {
            // Error Message to Connector
            cmdMessenger.sendCmd(kStatus, F("Custom Device does not fit in Memory"));
            return;
        }
        /* **********************************************************************************************
            Read the pins from the EEPROM, copy them into a buffer
            If you have set '"isI2C": true' in the device.json file, the first value is the I2C address
        ********************************************************************************************** */
        getStringFromEEPROM(adrPin, parameter);
        /* **********************************************************************************************
            Split the pins up into single pins. As the number of pins could be different between
            multiple devices, it is done here.
        ********************************************************************************************** */
        params = strtok_r(parameter, "|", &p);
        _pin1  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin2  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin3  = atoi(params);

        /* **********************************************************************************
            Read the configuration from the EEPROM, copy it into a buffer.
        ********************************************************************************** */
        getStringFromEEPROM(adrConfig, parameter);
        /* **********************************************************************************
            Split the config up into single parameter. As the number of parameters could be
            different between multiple devices, it is done here.
            This is just an example how to process the init string. Do NOT use
            "," or ";" as delimiter for multiple parameters but e.g. "|"
            For most customer devices it is not required.
            In this case just delete the following
        ********************************************************************************** */
        uint16_t Parameter1;
        char    *Parameter2;
        params     = strtok_r(parameter, "|", &p);
        Parameter1 = atoi(params);
        params     = strtok_r(NULL, "|", &p);
        Parameter2 = params;

        /* **********************************************************************************
            Next call the constructor of your custom device
            adapt it to the needs of your constructor
        ********************************************************************************** */
        // In most cases you need only one of the following functions
        // depending on if the constuctor takes the variables or a separate function is required
        _mydevice = new (allocateMemory(sizeof(MyCustomClass))) MyCustomClass(_pin1, _pin2);
        _mydevice->attach(Parameter1, Parameter2);
        // if your custom device does not need a separate begin() function, delete the following
        // or this function could be called from the custom constructor or attach() function
        _mydevice->begin();
        _initialized = true;
    } else if (_customType == MY_CUSTOM_DEVICE_2) {
        /* **********************************************************************************
            Check if the device fits into the device buffer
        ********************************************************************************** */
        if (!FitInMemory(sizeof(MyCustomClass))) {
            // Error Message to Connector
            cmdMessenger.sendCmd(kStatus, F("Custom Device does not fit in Memory"));
            return;
        }

        /* **********************************************************************************************
            Read the pins from the EEPROM, copy them into a buffer
            If you have set '"isI2C": true' in the device.json file, the first value is the I2C address
        ********************************************************************************************** */
        getStringFromEEPROM(adrPin, parameter);
        /* **********************************************************************************************
            split the pins up into single pins, as the number of pins could be different between
            multiple devices, it is done here
        ********************************************************************************************** */
        params = strtok_r(parameter, "|", &p);
        _pin1  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin2  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin3  = atoi(params);

        /* **********************************************************************************
            Read the configuration from the EEPROM, copy it into a buffer.
        ********************************************************************************** */
        getStringFromEEPROM(adrConfig, parameter);
        /* **********************************************************************************
            split the config up into single parameter. As the number of parameters could be
            different between multiple devices, it is done here.
            This is just an example how to process the init string. Do NOT use
            "," or ";" as delimiter for multiple parameters but e.g. "|"
            For most customer devices it is not required.
            In this case just delete the following
        ********************************************************************************** */
        uint16_t Parameter1;
        char    *Parameter2;
        params     = strtok_r(parameter, "|", &p);
        Parameter1 = atoi(params);
        params     = strtok_r(NULL, "|", &p);
        Parameter2 = params;

        /* **********************************************************************************
            Next call the constructor of your custom device
            adapt it to the needs of your constructor
        ********************************************************************************** */
        // In most cases you need only one of the following functions
        // depending on if the constuctor takes the variables or a separate function is required
        _mydevice = new (allocateMemory(sizeof(MyCustomClass))) MyCustomClass(_pin1, _pin2);
        _mydevice->attach(Parameter1, Parameter2);
        // if your custom device does not need a separate begin() function, delete the following
        // or this function could be called from the custom constructor or attach() function
        _mydevice->begin();
        _initialized = true;
    } else {
        cmdMessenger.sendCmd(kStatus, F("Custom Device is not supported by this firmware version"));
    }
}

/* **********************************************************************************
    The custom devives gets unregistered if a new config gets uploaded.
    Keep it as it is, mostly nothing must be changed.
    It gets called from CustomerDevice::Clear()
********************************************************************************** */
void MFCustomDevice::detach()
{
    _initialized = false;
    if (_customType == MY_CUSTOM_DEVICE_1) {
        _mydevice->detach();
    } else if (_customType == MY_CUSTOM_DEVICE_2) {
        _mydevice->detach();
    }
}

/* **********************************************************************************
    Within in loop() the update() function is called regularly
    Within the loop() you can define a time delay where this function gets called
    or as fast as possible. See comments in loop().
    It is only needed if you have to update your custom device without getting
    new values from the connector.
    Otherwise just make a return; in the calling function.
    It gets called from CustomerDevice::update()
********************************************************************************** */
void MFCustomDevice::update()
{
    if (!_initialized) return;
    /* **********************************************************************************
        Do something if required
    ********************************************************************************** */
    if (_customType == MY_CUSTOM_DEVICE_1) {
        _mydevice->update();
    } else if (_customType == MY_CUSTOM_DEVICE_2) {
        _mydevice->update();
    }
}

/* **********************************************************************************
    If an output for the custom device is defined in the connector,
    this function gets called when a new value is available.
    It gets called from CustomerDevice::OnSet()
********************************************************************************** */
void MFCustomDevice::set(int16_t messageID, char *setPoint)
{
    if (!_initialized) return;

    if (_customType == MY_CUSTOM_DEVICE_1) {
        _mydevice->set(messageID, setPoint);
    } else if (_customType == MY_CUSTOM_DEVICE_2) {
        _mydevice->set(messageID, setPoint);
    }
}
