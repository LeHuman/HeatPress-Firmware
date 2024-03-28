#pragma once

#include <stdint.h>

// Commands
static const uint8_t CMD_CLEARDISPLAY __attribute__((unused)) = 0x01;
static const uint8_t CMD_RETURNHOME __attribute__((unused)) = 0x02;
static const uint8_t CMD_ENTRYMODESET __attribute__((unused)) = 0x04;
static const uint8_t CMD_DISPLAYCONTROL __attribute__((unused)) = 0x08;
static const uint8_t CMD_CURSORSHIFT __attribute__((unused)) = 0x10;
static const uint8_t CMD_FUNCTIONSET __attribute__((unused)) = 0x20;
static const uint8_t CMD_SETCGRAMADDR __attribute__((unused)) = 0x40;
static const uint8_t CMD_SETDDRAMADDR __attribute__((unused)) = 0x80;

// Flags for display entry mode
static const uint8_t DISPLAY_ENTRYRIGHT __attribute__((unused)) = 0x00;
static const uint8_t DISPLAY_ENTRYLEFT __attribute__((unused)) = 0x02;
static const uint8_t DISPLAY_ENTRYSHIFTINCREMENT __attribute__((unused)) = 0x01;
static const uint8_t DISPLAY_ENTRYSHIFTDECREMENT __attribute__((unused)) = 0x00;

// Flags for display on/off control
static const uint8_t CTRL_DISPLAYON __attribute__((unused)) = 0x04;
static const uint8_t CTRL_DISPLAYOFF __attribute__((unused)) = 0x00;
static const uint8_t CTRL_CURSORON __attribute__((unused)) = 0x02;
static const uint8_t CTRL_CURSOROFF __attribute__((unused)) = 0x00;
static const uint8_t CTRL_BLINKON __attribute__((unused)) = 0x01;
static const uint8_t CTRL_BLINKOFF __attribute__((unused)) = 0x00;

// Flags for display/cursor shift
static const uint8_t SHIFT_DISPLAYMOVE __attribute__((unused)) = 0x08;
static const uint8_t SHIFT_CURSORMOVE __attribute__((unused)) = 0x00;
static const uint8_t SHIFT_MOVERIGHT __attribute__((unused)) = 0x04;
static const uint8_t SHIFT_MOVELEFT __attribute__((unused)) = 0x00;

// Flags for function set
static const uint8_t LCD_8BITMODE __attribute__((unused)) = 0x10;
static const uint8_t LCD_4BITMODE __attribute__((unused)) = 0x00;

static const uint8_t LCD_2LINE __attribute__((unused)) = 0x08;
static const uint8_t LCD_1LINE __attribute__((unused)) = 0x00;

// Char Sizes
static const uint8_t CHAR_5x10DOTS __attribute__((unused)) = 0x04;
static const uint8_t CHAR_5x8DOTS __attribute__((unused)) = 0x00;

// Flags for backlight control
static const uint8_t BL_BACKLIGHT __attribute__((unused)) = 0x08;
static const uint8_t BL_NOBACKLIGHT __attribute__((unused)) = 0x00;

static const uint8_t BIT_D7 __attribute__((unused)) = 0b10000000; // Data 7 bit
static const uint8_t BIT_D6 __attribute__((unused)) = 0b01000000; // Data 6 bit
static const uint8_t BIT_D5 __attribute__((unused)) = 0b00100000; // Data 5 bit
static const uint8_t BIT_D4 __attribute__((unused)) = 0b00010000; // Data 4 bit
static const uint8_t BIT_BL __attribute__((unused)) = 0b00001000; // Backlight bit
static const uint8_t BIT_EN __attribute__((unused)) = 0b00000100; // Enable bit
static const uint8_t BIT_RW __attribute__((unused)) = 0b00000010; // Read/Write bit
static const uint8_t BIT_RS __attribute__((unused)) = 0b00000001; // Register select bit