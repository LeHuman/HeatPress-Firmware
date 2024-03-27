#pragma once

#include <stdlib.h>

#include "main.h"

// Referenced Example: https://embeddedthere.com/interfacing-stm32-with-i2c-lcd-with-hal-code-example/

typedef struct PCF8574_LCD {
    I2C_HandleTypeDef *hi2c; // I2C STM32 Hal handle
    uint8_t backlight_state; // Define variable for backlight state

    uint8_t I2C_ADDR;        // I2C address of the PCF8574
    uint8_t LCD_ROWS;        // Number of rows on the LCD
    uint8_t LCD_COLS;        // Number of columns on the LCD

    uint32_t buffer_sz;      // Total size of buffer / number of characters
    uint8_t **buffer;        // Buffer matrix
    uint32_t update;         // update filter

    uint8_t RS_BIT;          // Register select bit
    uint8_t EN_BIT;          // Enable bit
    uint8_t BL_BIT;          // Backlight bit
    uint8_t D4_BIT;          // Data 4 bit
    uint8_t D5_BIT;          // Data 5 bit
    uint8_t D6_BIT;          // Data 6 bit
    uint8_t D7_BIT;          // Data 7 bit
} PCF8574_LCD;

PCF8574_LCD *pcf8574_lcd_new_default(I2C_HandleTypeDef *hi2c);
PCF8574_LCD *pcf8574_lcd_new_min(I2C_HandleTypeDef *hi2c, uint8_t I2C_ADDR, uint8_t LCD_ROWS, uint8_t LCD_COLS);
PCF8574_LCD *pcf8574_lcd_new(
    I2C_HandleTypeDef *hi2c,
    uint8_t I2C_ADDR, // 0x27
    uint8_t LCD_ROWS, // 2
    uint8_t LCD_COLS, // 16
    uint8_t RS_BIT,   // 0
    uint8_t EN_BIT,   // 2
    uint8_t BL_BIT,   // 3
    uint8_t D4_BIT,   // 4
    uint8_t D5_BIT,   // 5
    uint8_t D6_BIT,   // 6
    uint8_t D7_BIT    // 7
);

void pcf8574_lcd_free(PCF8574_LCD *lcd);

void pcf8574_lcd_initialize(PCF8574_LCD *lcd);
void pcf8574_lcd_clear(PCF8574_LCD *lcd);

void pcf8574_lcd_write_nibble(PCF8574_LCD *lcd, uint8_t nibble, uint8_t rs);
void pcf8574_lcd_send_cmd(PCF8574_LCD *lcd, uint8_t cmd);
void pcf8574_lcd_send_data(PCF8574_LCD *lcd, uint8_t data);
void pcf8574_lcd_write_string(PCF8574_LCD *lcd, char *str);
void pcf8574_lcd_set_cursor(PCF8574_LCD *lcd, uint8_t row, uint8_t column);
void pcf8574_lcd_backlight(PCF8574_LCD *lcd, uint8_t state);
