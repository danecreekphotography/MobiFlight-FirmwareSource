//
// MFBoards.h (Arduino Uno/Nano)
//
// (C) MobiFlight Project 2022
//

#ifndef MFBoardUno_h
#define MFBoardUno_h

#ifndef MF_SEGMENT_SUPPORT
#define MF_SEGMENT_SUPPORT 1
#endif
#ifndef MF_LCD_SUPPORT
#define MF_LCD_SUPPORT 1
#endif
#ifndef MF_STEPPER_SUPPORT
#define MF_STEPPER_SUPPORT 1
#endif
#ifndef MF_SERVO_SUPPORT
#define MF_SERVO_SUPPORT 1
#endif
#ifndef MF_ANALOG_SUPPORT
#define MF_ANALOG_SUPPORT 1
#endif
#ifndef MF_OUTPUT_SHIFTER_SUPPORT
#define MF_OUTPUT_SHIFTER_SUPPORT 1
#endif
#ifndef MF_INPUT_SHIFTER_SUPPORT
#define MF_INPUT_SHIFTER_SUPPORT 1
#endif
#ifndef MF_MUX_SUPPORT
#define MF_MUX_SUPPORT 1
#endif
#ifndef MF_DIGIN_MUX_SUPPORT
#define MF_MUX_SUPPORT       1
#define MF_DIGIN_MUX_SUPPORT 1
#endif

#define MOBIFLIGHT_TYPE     "MobiFlight Nano"
#define MOBIFLIGHT_SERIAL   "0987654321"
#define MOBIFLIGHT_NAME     "MobiFlight Nano"
#define EEPROM_SIZE         1024 // EEPROMSizeUno
#define MEMLEN_CONFIG       286  // max. size for config which wil be stored in EEPROM
#define MEMLEN_NAMES_BUFFER 220  // max. size for configBuffer, contains only names from inputs
#define MF_MAX_DEVICEMEM    420  // max. memory size for devices

#endif

// MFBoards.h