//
// CustomDevice.cpp
//
// (C) MobiFlight Project 2022
//

#include "commandmessenger.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include "CustomDevice.h"
#include "MFCustomDevice.h"
#if defined(USE_2ND_CORE) && defined(ARDUINO_ARCH_RP2040)
#include <FreeRTOS.h>
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
#if defined(USE_2ND_CORE)
    char payload[SERIAL_RX_BUFFER_SIZE];
#endif

    bool setupArray(uint16_t count)
    {
        if (!FitInMemory(sizeof(MFCustomDevice) * count))
            return false;
        customDevice     = new (allocateMemory(sizeof(MFCustomDevice) * count)) MFCustomDevice();
        maxCustomDevices = count;
        return true;
    }

    void Add(uint16_t adrPin, uint16_t adrType, uint16_t adrConfig, bool configFromFlash)
    {
        if (customDeviceRegistered == maxCustomDevices)
            return;
        customDevice[customDeviceRegistered] = MFCustomDevice();
        customDevice[customDeviceRegistered].attach(adrPin, adrType, adrConfig, configFromFlash);
        customDeviceRegistered++;
#if defined(DEBUG2CMDMESSENGER)
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
#if defined(DEBUG2CMDMESSENGER)
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
#if !defined(USE_2ND_CORE)
        for (int i = 0; i != customDeviceRegistered; i++) {
            customDevice[i].update();
        }
#endif
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
#if defined(USE_2ND_CORE) && defined(ARDUINO_ARCH_RP2040)
        // copy the message, could get be overwritten from the next message while processing on 2nd core
        strncpy(payload, output, SERIAL_RX_BUFFER_SIZE);
        // wait for 2nd core
        rp2040.fifo.pop();
        // Hmhm, how to get the function pointer to a function from class??
        // rp2040.fifo.push((uintptr_t) &customDevice[device].set);
        rp2040.fifo.push(device);
        rp2040.fifo.push(messageID);
        rp2040.fifo.push((uint32_t)&payload);
#else
        customDevice[device].set(messageID, output); // send the string to your custom device
#endif
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
                customDevice[i].set(MESSAGEID_POWERSAVINGMODE, (char *)"1");
            else
                customDevice[i].set(MESSAGEID_POWERSAVINGMODE, (char *)"0");
        }
    }

#if defined(USE_2ND_CORE) && defined(ARDUINO_ARCH_RP2040)
    void stopUpdate2ndCore(bool stop)
    {
        // wait for 2nd core
        rp2040.fifo.pop();
        // send command to stop/start updating to 2nd core
        // negative device numbers are for commands to 2nd core
        rp2040.fifo.push(START_STOP_2ND_CORE);
        // send stop/start
        rp2040.fifo.push(stop);
        // communication is always done using 3 messages
        rp2040.fifo.push(0);
        // wait for execution of command
        rp2040.fifo.pop();
    }
#endif
} // end of namespace

#if defined(USE_2ND_CORE) && defined(ARDUINO_ARCH_RP2040)
/* **********************************************************************************
    This will run the set() function from the custom device on the 2nd core
    Be aware NOT to use the function calls from the Pico SDK!
    Only use the functions from the used framework from EarlePhilHower
    If you mix them up it will give undefined behaviour and strange effects
    see https://arduino-pico.readthedocs.io/en/latest/multicore.html
********************************************************************************** */
void setup1()
{
    // send "ready" message to 1st core
    rp2040.fifo.push(true);
}

void loop1()
{
    int16_t device;
    int16_t messageID;
    char   *payload;
    bool    stopUpdating = false;
#if defined(DEBUG2CMDMESSENGER)
    uint32_t lastMillis = 0;
#endif

    while (1) {
#if defined(DEBUG2CMDMESSENGER)
        if (millis() - lastMillis >= MF_CUSTOMDEVICE_POLL_MS) {
#endif
#if defined(MF_CUSTOMDEVICE_HAS_UPDATE)
            for (int i = 0; i < CustomDevice::customDeviceRegistered && !stopUpdating; i++) {
                CustomDevice::customDevice[i].update();
            }
#endif
#if defined(DEBUG2CMDMESSENGER)
            lastMillis = millis();
        }
#endif
        if (rp2040.fifo.available() == 3) {
            // Hmhm, how to get the function pointer to a function from class??
            // int32_t (*func)(int16_t, char*) = (int32_t(*)(int16_t, char*)) rp2040.fifo.pop();
            device    = (int16_t)rp2040.fifo.pop();
            messageID = (int16_t)rp2040.fifo.pop();
            payload   = (char *)rp2040.fifo.pop();
            if (device == -1) {
                stopUpdating = (bool)messageID;
                // inform core 0 that command has been executed
                // it's additional needed in this case
                rp2040.fifo.push(true);
            } else {
                // (*func)(messageID, payload);
                CustomDevice::customDevice[device].set(messageID, payload);
            }
            // send ready for next message to 1st core
            rp2040.fifo.push(true);
        }
    }
}
#endif
