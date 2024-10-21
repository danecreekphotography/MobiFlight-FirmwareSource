//
// MFLCDDisplay.cpp
//
// (C) MobiFlight Project 2022
//

#include "MFLCDDisplay.h"

MFLCDDisplay::MFLCDDisplay()
{
    _initialized = false;
}

void MFLCDDisplay::display(const char *string)
{
    if (!_initialized)
        return;
    for (uint8_t line = 0; line != _lines; line++) {
        _lcdDisplay.setCursor(0, line);
        _lcdDisplay.writeString(&string[line * _cols], _cols);
    }
}

void MFLCDDisplay::attach(byte address, byte cols, byte lines)
{
    _address     = address;
    _cols        = cols;
    _lines       = lines;
    _initialized = true;
    _lcdDisplay.init((uint8_t)address, (uint8_t)cols, (uint8_t)lines);
    _lcdDisplay.backlight();
    Wire.setClock(400000);
    test();
}

void MFLCDDisplay::detach()
{
    if (!_initialized)
        return;
    _initialized = false;
}

void MFLCDDisplay::powerSavingMode(bool state)
{
    if (state) {
        _lcdDisplay.noBacklight();
        _lcdDisplay.noDisplay();
    }
    else {
        _lcdDisplay.backlight();
        _lcdDisplay.display();
    }
}

void MFLCDDisplay::test()
{
    if (!_initialized)
        return;
    uint8_t preLines = 0;
    _lcdDisplay.clear();

    if (_lines > 2) {
        preLines = (_lines / 2) - 1; // floor needs much Flash and for integer it's the same
    }

    _printCentered("MobiFlight", preLines++);
    if (_lines > 1) {
        _printCentered("Rocks!", preLines++);
    }

    _lcdDisplay.setCursor(0, 0);
}

void MFLCDDisplay::_printCentered(const char *str, uint8_t line)
{
    uint8_t startCol  = 0;
    uint8_t printChar = _cols;

    if (_cols > strlen(str)) {
        startCol  = (_cols - strlen(str)) / 2;
        printChar = strlen(str);
    }

    _lcdDisplay.setCursor(startCol, line);

    for (uint8_t i = 0; i < printChar; i++) {
        _lcdDisplay.write(str[i]);
    }
}

// MFLCDDisplay.cpp
