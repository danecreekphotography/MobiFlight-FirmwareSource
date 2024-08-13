//
// Stepper.h
//
// (C) MobiFlight Project 2022
//

#pragma once

namespace Stepper
{
    bool setupArray(uint16_t count);
    void Add(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t btnPin1, uint8_t mode = 0, int8_t backlash = 0, bool deactivateOutput = false);
    void Clear();
    void OnSet();
    void OnReset();
    void OnSetZero();
    void update();
    void OnSetSpeedAccel();
    void PowerSave(bool state);
#if defined(STEPPER_ON_2ND_CORE)
    void stopUpdate2ndCore(bool stop);
#endif
}

// Stepper.h