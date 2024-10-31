//
// LedControl_dual.cpp
//
// (C) MobiFlight Project 2023
//

#include "LedControl_dual.h"
#include "allocateMem.h"

// Segments to be switched on for characters and digits on 7-Segment Displays
// bit/segment sequence: dABCDEFG
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---   .d
//      D

const static byte charTable[128] PROGMEM = {
    // 0         ,1         ,2         ,3         ,4         ,5         ,6         ,7
    0b01111110, 0b00110000, 0b01101101, 0b01111001, 0b00110011, 0b01011011, 0b01011111, 0b01110000,
    // 8         ,9         ,A         ,B         ,C         ,D         ,E         ,F
    0b01111111, 0b01111011, 0b01110111, 0b00011111, 0b00001101, 0b00111101, 0b01001111, 0b01000111,
    // <0x10>    ,<0x11>    ,<0x12>    ,<0x13>    ,<0x14>    ,<0x15>    ,<0x16>    ,<0x17>
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // <0x18>    ,<0x19>    ,<0x1A>    ,<0x1B>    ,<0x1C>    ,<0x1D>    ,<0x1E>    ,<0x1F>
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // (space)    ,!          ,"          ,#          ,$          ,%          ,&          ,'
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // (         ,)         ,*         ,+         ,,         ,-         ,.         ,/
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b10000000, 0b00000001, 0b10000000, 0b00100101,
    // 0         ,1         ,2         ,3         ,4         ,5         ,6         ,7
    0b01111110, 0b00110000, 0b01101101, 0b01111001, 0b00110011, 0b01011011, 0b01011111, 0b01110000,
    // 8         ,9         ,:         ,;         ,<         ,=         ,>         ,?
    0b01111111, 0b01111011, 0b00000000, 0b00000000, 0b00000000, 0b00001001, 0b00000000, 0b00000000,
    // @         ,A         ,0b         ,C         ,D         ,E         ,F         ,G
    0b00000000, 0b01110111, 0b01111111, 0b01001110, 0b01111110, 0b01001111, 0b01000111, 0b01011111,
    // H         ,I         ,J         ,K         ,L         ,M         ,N         ,O
    0b00110111, 0b00110000, 0b01111100, 0b00000000, 0b00001110, 0b00000000, 0b01110110, 0b01111110,
    // P         ,Q         ,R         ,S         ,T         ,U         ,V         ,W
    0b01100111, 0b01111110, 0b01110111, 0b01011011, 0b01000110, 0b00111110, 0b00000000, 0b00000000,
    // X         ,Y         ,Z         ,[         ,\         ,]         ,^         ,_
    0b00000000, 0b00110011, 0b01101101, 0b01001110, 0b00010011, 0b01111000, 0b00000000, 0b00001000,
    // `         ,a         ,0b         ,c         ,d         ,e         ,f         ,g
    0b00000000, 0b01111101, 0b00011111, 0b00001101, 0b00111101, 0b01001111, 0b01000111, 0b01111011,
    // h         ,i         ,j         ,k         ,l         ,m         ,n         ,o
    0b00110111, 0b00110000, 0b00111000, 0b00000000, 0b00000110, 0b00000000, 0b00010101, 0b00011101,
    // p         ,q         ,r         ,s         ,t         ,u         ,v         ,w
    0b01100111, 0b01110011, 0b00000101, 0b01011011, 0b00001111, 0b00011100, 0b00000000, 0b00000000,
    // x         ,y         ,z         ,{         ,|         ,}         ,~         ,(delete)
    0b00000000, 0b00100111, 0b01101101, 0b01001110, 0b00000000, 0b01111000, 0b00000000, 0b00000000};

// =======================================================================
// MAX72xx Definitions
// =======================================================================

// the opcodes for the MAX7221 and MAX7219
enum {
    OP_NOOP = 0,
    OP_DIGIT0,
    OP_DIGIT1,
    OP_DIGIT2,
    OP_DIGIT3,
    OP_DIGIT4,
    OP_DIGIT5,
    OP_DIGIT6,
    OP_DIGIT7,
    OP_DECODEMODE,
    OP_INTENSITY,
    OP_SCANLIMIT,
    OP_SHUTDOWN,
    OP_DISPLAYTEST = 15
};

#if defined(LEDCONTROL_NO_BUF)
uint8_t *LedControl::rawdata;
#endif

// =======================================================================
// TM1637 Definitions
// =======================================================================

// TM1637 Commands
//
// Communication Sequence (Automatic Address)
//     Cmd1: Start + CmdSetData + ACK + Stop +
//     Cmd2+Data: Start + CmdSetAddress + ACK + (Data + ACK) * N +
//     Cmd3: Start + CmdDisplay + ACK + Stop
//
// Communication Sequence (Fixed Address)
//     Cmd1: Start + CmdSetData + ACK + Stop
//     Cmd2+Data: (Start + Command + ACK + Data + ACK) * N + Stop +
//     Cmd3: Start + Command3 + ACK + Stop
//
// CmdSetData - Data command settings (byte) - TM1637_I2C_COMM1
// B7 B6 B5 B4 B3 B2 B1 B0 - Function Description
// 0  1  0  0  _  _  0  0  - (Data read/write) Write data to display register
// 0  1  0  0  _  _  1  0  - (Data read/write) Read key scan data
// 0  1  0  0  _  0  _  _  - (Address mode) Automatic address adding
// 0  1  0  0  _  1  _  _  - (Address mode) Fixed address
// 0  1  0  0  0  _  _  _  - (Test mode) Normal mode
// 0  1  0  0  1  _  _  _  - (Test mode) Test mode
//
// CmdSetAddress - Set Address - Digit (byte) - TM1637_I2C_COMM2
// B7 B6 B5 B4 B3 B2 B1 B0 - Function Description
// 1  1  0  0  0  0  0  0  - Digit 1 - C0H Grid1
// 1  1  0  0  0  0  0  1  - Digit 2 - C1H Grid2
// 1  1  0  0  0  0  1  0  - Digit 3 - C2H Grid3
// 1  1  0  0  0  0  1  1  - Digit 4 - C3H Grid4
// 1  1  0  0  0  1  0  0  - Digit 5 - C4H Grid5
// 1  1  0  0  0  1  0  1  - Digit 6 - C5H Grid6
//
// CmdDisplay - Set Display - Digit (byte) - TM1637_I2C_COMM3
// B7 B6 B5 B4 B3 B2 B1 B0 - Function Description
// 1  0  0  0  _  0  0  0  - Brightness - Pulse width is set as 1/16
// 1  0  0  0  _  0  0  1  - Brightness - Pulse width is set as 2/16
// 1  0  0  0  _  0  1  0  - Brightness - Pulse width is set as 4/16
// 1  0  0  0  _  0  1  1  - Brightness - Pulse width is set as 10/16
// 1  0  0  0  _  1  0  0  - Brightness - Pulse width is set as 11/16
// 1  0  0  0  _  1  0  1  - Brightness - Pulse width is set as 12/16
// 1  0  0  0  _  1  1  0  - Brightness - Pulse width is set as 13/16
// 1  0  0  0  _  1  1  1  - Brightness - Pulse width is set as 14/16
// 1  0  0  0  0  _  _  _  - Display OFF
// 1  0  0  0  1  _  _  _  - Display ON

#define TM1637_I2C_COMM1  0x40 // CmdSetData       0b01000000
#define TM1637_I2C_COMM2  0xC0 // CmdSetAddress    0b11000000
#define TM1637_I2C_COMM3  0x80 // CmdDisplay       0b10000000
#define TM1637_I2C_COMM1F 0x44 // CmdSetData - fixedAddress    0b11000100
// =======================================================================

// Digit sequence map for 6 digit displays
const uint8_t digitmap[] = {2, 1, 0, 5, 4, 3};

bool LedControl::begin(uint8_t type, uint8_t dataPin, uint8_t clkPin, uint8_t csPin, uint8_t numDevices)
{
    _type    = type;
    _dataPin = dataPin;
    _clkPin  = clkPin;
    _csPin   = csPin;

    if (!FitInMemory(sizeof(uint8_t) * numDevices * 2))
        return false;
    rawdata = new (allocateMemory(sizeof(uint8_t) * numDevices * 2)) uint8_t;

    if (isMAX()) {
        // make sure we have max 8 chips in the daisy chain
        if (numDevices > 8) numDevices = 8;

        if (!FitInMemory(sizeof(uint8_t) * numDevices * 8))
            return false;
        digitBuffer = new (allocateMemory(sizeof(uint8_t) * numDevices * 8)) uint8_t;
        maxUnits    = numDevices;
        pinMode(_dataPin, OUTPUT);
        pinMode(_clkPin, OUTPUT);
        pinMode(_csPin, OUTPUT);
        digitalWrite(_csPin, HIGH);
        for (uint8_t i = 0; i < maxUnits; i++) {
            spiTransfer(i, OP_DISPLAYTEST, 0);
            setScanLimit(i, 7);               // scanlimit is set to max on startup
            spiTransfer(i, OP_DECODEMODE, 0); // decode is done in source
            clearDisplay(i);
            shutdown(i, true); // we go into shutdown-mode on startup
        }
    } else {
        maxUnits = (this->_type == LedSegment::TYPE_TM1637_4DIGITS ? 4 : 6);
        // Both pins are set as inputs, allowing the pull-up resistors to pull them up
        pinMode(_clkPin, INPUT_PULLUP);
        pinMode(_dataPin, INPUT_PULLUP);
        digitalWrite(_clkPin, LOW);  // Prepare '0' value as dominant
        digitalWrite(_dataPin, LOW); // Prepare '0' value as dominant
        clearDisplay(0);
        // setIntensity(0, MAX_BRIGHTNESS);
        brightness = MAX_BRIGHTNESS;
        shutdown(0, true);
    }

    return true;
}

void LedControl::shutdown(uint8_t addr, bool b)
{
    if (isMAX()) {
        if (addr >= maxUnits) return;
        spiTransfer(addr, OP_SHUTDOWN, b ? 0 : 1);
    } else {
        uint8_t bri = brightness >> 1;
        if (!b) bri |= 0x08;
        // Write COMM3 + intensity
        start();
        writeByte(TM1637_I2C_COMM3 + bri);
        stop();
    }
}

void LedControl::setIntensity(uint8_t addr, uint8_t intensity)
{
    if (intensity > 15) intensity = 15;
    brightness = intensity;
    if (isMAX()) {
        if (addr >= maxUnits) return;
        spiTransfer(addr, OP_INTENSITY, brightness);
    } else {
        if (intensity > 0) {
            if (intensity > 1) intensity >>= 1;
            intensity |= 0x08;
        }
        // Write COMM3 + intensity
        start();
        writeByte(TM1637_I2C_COMM3 + intensity);
        stop();
    }
}

void LedControl::clearDisplay(uint8_t addr)
{
    if (isMAX()) {
        if (addr >= maxUnits) return;
        for (uint8_t i = 0; i < 8; i++) {
            spiTransfer(addr, i + 1, 0);
        }
    } else {
#if defined(LEDCONTROL_NO_BUF)
        for (uint8_t i = 0; i < 8; i++) {
            writeOneDigit(i, 0);
        }
#else
        memset(rawdata, 0, maxUnits);
        writeBuffer();
#endif
    }
}

void LedControl::setDigit(uint8_t addr, uint8_t digit, uint8_t value, bool dp, bool sendNow)
{
    if (addr >= maxUnits) return;
    if ((value > 15) && (value != '-')) value = (uint8_t)' '; // Use space for invalid digit
    if (dp) value |= 0x80;
    setPattern(addr, digit, value, sendNow);
}

void LedControl::setChar(uint8_t addr, uint8_t digit, char value, bool dp, bool sendNow)
{
    uint8_t v;
    if (addr >= maxUnits) return;
    v = (uint8_t)value;  // Get rid of signedness
    if (v > 127) v = 32; // undefined: replace with space char
    if (dp) v |= 0x80;
    setPattern(addr, digit, v, sendNow);
}

void LedControl::setSingleSegment(uint8_t subModule, uint8_t segment, uint8_t value, bool sendNow)
{
    uint8_t digit       = segment >> 3;
    uint8_t bitPosition = segment % 8;
    uint8_t offset      = subModule * 8;

    if (isMAX()) {
        if (subModule >= maxUnits) return;
        if (segment > 63) return;
        if (value) {
            digitBuffer[offset + digit] |= (1 << bitPosition);
        } else {
            digitBuffer[offset + digit] &= ~(1 << bitPosition);
        }
        spiTransfer(subModule, digit + 1, digitBuffer[offset + digit]);
    } else {
        if (subModule >= maxUnits) return;
        if (segment >= maxUnits * 8) return;
        // Same order as MAX72XX
        // MAX72XX order is:      dABCDEFG
        // TM1637 order required: ABCDEFGd
        bitPosition++;
        if (bitPosition == 8)
            bitPosition = 0;
        if (value) {
            rawdata[(maxUnits - 1) - digit] |= (1 << bitPosition);
        } else {
            rawdata[(maxUnits - 1) - digit] &= ~(1 << bitPosition);
        }
        if (sendNow) writeDigits(digit, 1);
    }
}

void LedControl::setPattern(uint8_t addr, uint8_t digit, uint8_t value, bool sendNow)
{
    if (digit > getDigitCount() - 1) return;
    uint8_t v;
    v = pgm_read_byte_near(charTable + (value & 0x7F));
    if (isMAX()) {
        uint8_t offset = addr * 8;
        if (value & 0x80) v |= 0x80;
        digitBuffer[offset + digit] = v;
        spiTransfer(addr, digit + 1, v); // Always send immediately for MAX
    } else {
        // Original data for MAX has the bit sequence: dABCDEFG
        // Common TM1637 boards are connected so that they require: dGFEDCBA
        // For the least effort, we alter the byte to be transmitted as: ABCDEFGd
        // and then just transmit them reversed (from LSb to MSb)
        v <<= 1;
        if (value & 0x80) v |= 0x01;
#if defined(LEDCONTROL_NO_BUF)
        writeOneDigit(digit, v);
#else
        rawdata[(maxUnits - 1) - digit] = v; // Change only the individual affected digit in static buffer
        if (sendNow) {
            writeDigits(digit, 1);
        }
#endif
    }
}

// ------------------------------------------------
// MAX-specific driver methods
// ------------------------------------------------

void LedControl::setScanLimit(uint8_t addr, uint8_t limit)
{
    if (!isMAX()) return;
    if (addr >= maxUnits) return;
    if (limit > 7) return;
    spiTransfer(addr, OP_SCANLIMIT, limit);
}

void LedControl::spiTransfer(uint8_t addr, uint8_t opcode, uint8_t data)
{
    uint8_t offset   = addr * 2;
    uint8_t maxBytes = maxUnits * 2;

    // for (uint8_t i = 0; i < maxBytes; i++) rawdata[i] = (byte)0;
    memset(rawdata, 0, maxBytes);
    rawdata[offset + 1] = opcode;
    rawdata[offset]     = data;

    digitalWrite(_csPin, LOW);
    for (uint8_t i = maxBytes; i > 0; i--) {
        // shiftOut(IO_DTA, IO_CLK, MSBFIRST, rawdata[i - 1]);
        byte dta = rawdata[i - 1];
        for (uint8_t m = 0x80; m != 0; m >>= 1) {
            // MSB first
            digitalWrite(_dataPin, (dta & m));
            digitalWrite(_clkPin, HIGH);
            digitalWrite(_clkPin, LOW);
        }
    }
    digitalWrite(_csPin, HIGH);
}

// ------------------------------------------------
// TM-specific driver methods
// ------------------------------------------------

void LedControl::start()
{
    pinMode(_dataPin, OUTPUT);
    bitDelay();
}

void LedControl::stop()
{
    pinMode(_dataPin, OUTPUT);
    bitDelay();
    pinMode(_clkPin, INPUT);
    bitDelay();
    pinMode(_dataPin, INPUT);
    bitDelay();
}

bool LedControl::writeByte(uint8_t data, bool rvs)
{
    uint8_t msk = (rvs ? 0x80 : 0x01);
    for (uint8_t i = 0; i < 8; i++) {
        // CLK low
        pinMode(_clkPin, OUTPUT);
        bitDelay();
        // Set data bit
        pinMode(_dataPin, (data & msk) ? INPUT : OUTPUT);
        bitDelay();
        // CLK high
        pinMode(_clkPin, INPUT);
        bitDelay();
        data = (rvs ? data << 1 : data >> 1);
    }
    // Wait for acknowledge
    // CLK to zero
    pinMode(_clkPin, OUTPUT);
    pinMode(_dataPin, INPUT);
    bitDelay();
    // CLK to high
    pinMode(_clkPin, INPUT);
    bitDelay();
    uint8_t ack = digitalRead(_dataPin);
    if (ack == 0) pinMode(_dataPin, OUTPUT);
    bitDelay();
    pinMode(_clkPin, OUTPUT);
    bitDelay();
    return ack;
}

#if defined(LEDCONTROL_NO_BUF)

void LedControl::writeOneDigit(uint8_t ndigit, uint8_t pattern)
{
    uint8_t b;
    // Write COMM1
    start();
    writeByte(TM1637_I2C_COMM1F); // TM1637_I2C_COMM1 is also fine
    stop();

    start();
    ndigit = (maxUnits - 1) - ndigit;
    b      = ((maxUnits == 4) ? ndigit : digitmap[ndigit]);
    writeByte(TM1637_I2C_COMM2 + b);
    // Write only raw data bit-reversed (to use the existing data in MAX-format)
    writeByte(pattern, true);
    stop();
}

#endif

// =========================================================
//   Methods for extended library
// =========================================================

#ifndef LEDCONTROL_NO_BUF

void LedControl::writeDigits(uint8_t startd, uint8_t len)
{
    bool    is4Digit = (maxUnits == 4);
    uint8_t b;

    // Write COMM1
    start();
    writeByte(TM1637_I2C_COMM1);
    stop();

    uint8_t pos = (maxUnits - 1) - startd;
    b           = (is4Digit ? pos : digitmap[pos + len - 1]);

    start();
    writeByte(TM1637_I2C_COMM2 + b);
    // Write the data bytes
    if (pos + len > maxUnits) len = maxUnits - pos;
    uint8_t k;
    for (b = 0; b < len; b++) {
        k = (is4Digit ? b : len - b - 1);
        writeByte(rawdata[pos + k], true);
    }
    stop();
}

#endif

#if defined(LEDCONTROL_EXTENDED)

void LedControl::showNumber(uint8_t addr, int32_t num, bool isHex, uint8_t dots, bool leading_zero, uint8_t roffset)
{
    uint8_t digits[8];
    uint8_t pos;
    uint8_t maxlen        = getDigitCount();
    bool    minusRequired = (num < 0);
    if (minusRequired) num = -num;

    for (uint8_t i = 0; i < 8; i++)
        digits[i] = (uint8_t)' ';

    pos = (maxlen - 1) - roffset; // Reverse to use as counter
    if (num == 0) {
        if (leading_zero) {
            for (uint8_t i = 0; i < pos; i++) {
                digits[i] = '0';
            }
        }
        digits[pos] = '0';
    } else {
        uint8_t dval;
        do {
            if (num == 0) {
                if (leading_zero) {
                    digits[pos] = '0';
                    if (minusRequired && pos == 0) digits[0] = '-';
                } else {
                    if (minusRequired) digits[pos] = '-';
                    pos = 0;
                }
            } else {
                if (!isHex) {
                    dval        = num % 10;
                    digits[pos] = '0' + dval;
                } else {
                    dval        = num & 0x0F;
                    digits[pos] = ((dval > 9) ? 'A' - 10 : '0') + dval;
                }
            }
            if (!isHex) {
                num /= 10;
            } else {
                num >>= 4;
            }
        } while ((pos--) > 0);
    }
    showString(addr, (char *)digits, 0, dots);
}

void LedControl::showString(uint8_t addr, char *s, uint8_t loffset, uint8_t dots)
{
    uint8_t maxlen = getDigitCount();
    uint8_t msk    = 0x80 >> loffset;
    for (uint8_t d = loffset; d < maxlen && (*s != 0); d++) {
        uint8_t pos = (maxlen - 1) - d;
        setChar(addr, pos, *s++, ((dots & msk) != 0), false);
        msk >>= 1;
    }

#ifndef LEDCONTROL_NO_BUF
    if (!isMAX()) writeBuffer();
#endif
}

#endif
// end