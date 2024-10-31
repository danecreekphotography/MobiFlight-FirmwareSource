//
// allocatemem.cpp
//
// (C) MobiFlight Project 2022
//

#include "allocateMem.h"
#include "MFBoards.h"
#include "commandmessenger.h"

#if defined (ARDUINO_ARCH_AVR)
uint8_t     deviceBuffer[MF_MAX_DEVICEMEM] = {0};
#else
std::size_t    deviceBuffer[MF_MAX_DEVICEMEM] = {0};
#endif

uint16_t nextPointer                    = 0;

#if defined (ARDUINO_ARCH_AVR)
uint8_t     *allocateMemory(uint16_t size)
#else
std::size_t    *allocateMemory(uint16_t size)
#endif
{
    uint16_t actualPointer = nextPointer;
    nextPointer            = actualPointer + size;
    if (nextPointer >= MF_MAX_DEVICEMEM) {
        cmdMessenger.sendCmd(kStatus, F("DeviceBuffer Overflow!"));
        return nullptr;
    }
#ifdef DEBUG2CMDMESSENGER
    cmdMessenger.sendCmdStart(kDebug);
    cmdMessenger.sendCmdArg(F("Bytes added"));
    cmdMessenger.sendCmdArg(size);
    cmdMessenger.sendCmdEnd();
    cmdMessenger.sendCmdStart(kDebug);
    cmdMessenger.sendCmdArg(F("BufferUsage"));
    cmdMessenger.sendCmdArg(nextPointer);
    cmdMessenger.sendCmdEnd();
#endif
    return &deviceBuffer[actualPointer];
}

void ClearMemory()
{
    nextPointer = 0;
}

uint16_t GetAvailableMemory()
{
    return MF_MAX_DEVICEMEM - nextPointer;
}

bool FitInMemory(uint16_t size)
{
    if (nextPointer + size > MF_MAX_DEVICEMEM)
        return false;
    return true;
}

// allocatemem.cpp
