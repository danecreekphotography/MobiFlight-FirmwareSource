#pragma once

namespace CustomDevice
{
    bool setupArray(uint16_t count);
    void Add(uint16_t adrPin, uint16_t adrType, uint16_t adrConfig, bool configFromFlash);
    void Clear();
    void update();
    void OnSet();
    void PowerSave(bool state);
}