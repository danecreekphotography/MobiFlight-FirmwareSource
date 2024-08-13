#pragma once

namespace CustomDevice
{
    #define START_STOP_2ND_CORE -1

    bool setupArray(uint16_t count);
    void Add(uint16_t adrPin, uint16_t adrType, uint16_t adrConfig, bool configFromFlash);
    void Clear();
    void update();
    void OnSet();
    void PowerSave(bool state);
#if defined(STEPPER_ON_2ND_CORE)
    void stopUpdate2ndCore(bool stop);
#endif
}