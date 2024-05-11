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

} // end of namespace
