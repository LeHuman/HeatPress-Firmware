#pragma once

#include <stdlib.h>

#include "main.h"

// Referenced Example: https://github.com/lucasmaziero/LiquidCrystal_I2C
// Referenced Example: https://embeddedthere.com/interfacing-stm32-with-i2c-lcd-with-hal-code-example/

typedef enum PCF8574_lcd_format {
    LCD_5x10DOTS = 0x04,
    LCD_5x8DOTS = 0x00,
} PCF8574_lcd_format;

typedef struct PCF8574_LCD {
    I2C_HandleTypeDef *hi2c; // I2C STM32 Hal handle
    uint8_t address;         // Address

    struct display {
        uint8_t function;    // the set display function
        uint8_t control;     // the set display control
        uint8_t mode;        // the set display mode
    } display;               // Display struct

    uint8_t char_format;     // The character dot format that the display uses
    uint8_t backlight;       // Whether the backlight is enabled

    uint8_t columns;         // The number of columns the display has
    uint8_t rows;            // The number of rows the display has
    uint32_t characters;     // Total number of characters

} PCF8574_LCD;

PCF8574_LCD *pcf8574_lcd_new_default(I2C_HandleTypeDef *hi2c); // 0x27, 2, 16, LCD_5x8DOTS
PCF8574_LCD *pcf8574_lcd_new(I2C_HandleTypeDef *hi2c, uint8_t addr, uint8_t rows, uint8_t columns, PCF8574_lcd_format char_format);

void pcf8574_lcd_free(PCF8574_LCD *lcd);

void pcf8574_lcd_initialize(PCF8574_LCD *lcd);
void pcf8574_lcd_clear(PCF8574_LCD *lcd);

void pcf8574_lcd_write_nibble(PCF8574_LCD *lcd, uint8_t nibble, uint8_t rs);
void pcf8574_lcd_send_cmd(PCF8574_LCD *lcd, uint8_t cmd);
void pcf8574_lcd_send_data(PCF8574_LCD *lcd, uint8_t data);
void pcf8574_lcd_write_string(PCF8574_LCD *lcd, char *str);
void pcf8574_lcd_set_cursor(PCF8574_LCD *lcd, uint8_t row, uint8_t column);
void pcf8574_lcd_backlight(PCF8574_LCD *lcd, uint8_t state);
void pcf8574_lcd_create_char(PCF8574_LCD *lcd, uint8_t location, const uint8_t charmap[8]);
void pcf8574_lcd_fling_char(PCF8574_LCD *lcd, uint8_t charmap[8], uint8_t row, uint8_t column);
