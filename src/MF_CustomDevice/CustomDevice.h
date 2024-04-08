#pragma once

namespace CustomDevice
{
#define FUNCTION_ONSET    1
#define FUNCTION_UPDATE 2

    bool setupArray(uint16_t count);
    void Add(uint16_t adrPin, uint16_t adrType, uint16_t adrConfig);
    void Clear();
    void update();
    void OnSet();
    void PowerSave(bool state);
}