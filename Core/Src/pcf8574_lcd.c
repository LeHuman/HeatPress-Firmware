#include "pcf8574_lcd.h"

#include <stdint.h>
#include <stdlib.h>

PCF8574_LCD *pcf8574_lcd_new(I2C_HandleTypeDef *hi2c,
                             uint8_t I2C_ADDR,
                             uint8_t LCD_ROWS,
                             uint8_t LCD_COLS,
                             uint8_t RS_BIT,
                             uint8_t EN_BIT,
                             uint8_t BL_BIT,
                             uint8_t D4_BIT,
                             uint8_t D5_BIT,
                             uint8_t D6_BIT,
                             uint8_t D7_BIT) {

    PCF8574_LCD *lcd = calloc(1, sizeof(PCF8574_LCD));
    if (lcd != NULL) {
        lcd->hi2c = hi2c;
        lcd->backlight_state = 1;
        lcd->I2C_ADDR = I2C_ADDR;
        lcd->LCD_ROWS = LCD_ROWS;
        lcd->LCD_COLS = LCD_COLS;
        lcd->buffer_sz = LCD_ROWS * LCD_COLS;
        lcd->buffer = calloc(lcd->buffer_sz, sizeof(uint8_t));
        lcd->RS_BIT = RS_BIT;
        lcd->EN_BIT = EN_BIT;
        lcd->BL_BIT = BL_BIT;
        lcd->D4_BIT = D4_BIT;
        lcd->D5_BIT = D5_BIT;
        lcd->D6_BIT = D6_BIT;
        lcd->D7_BIT = D7_BIT;
    }

    return lcd;
}

uint32_t hash(PCF8574_LCD *lcd, int x, int y) {
    return (x ^ y) % 32;
}

PCF8574_LCD *pcf8574_lcd_new_min(I2C_HandleTypeDef *hi2c, uint8_t I2C_ADDR, uint8_t LCD_ROWS, uint8_t LCD_COLS) {
    return pcf8574_lcd_new(hi2c, I2C_ADDR, LCD_ROWS, LCD_COLS, 0, 2, 3, 4, 5, 6, 7);
}

PCF8574_LCD *pcf8574_lcd_new_default(I2C_HandleTypeDef *hi2c) {
    return pcf8574_lcd_new_min(hi2c, 0x27, 2, 16);
}

void pcf8574_lcd_free(PCF8574_LCD *lcd) {
    free(lcd->buffer);
    free(lcd);
}

void pcf8574_lcd_initialize(PCF8574_LCD *lcd) {
    HAL_Delay(50);
    pcf8574_lcd_write_nibble(lcd, 0x03, 0);
    HAL_Delay(5);
    pcf8574_lcd_write_nibble(lcd, 0x03, 0);
    HAL_Delay(1);
    pcf8574_lcd_write_nibble(lcd, 0x03, 0);
    HAL_Delay(1);
    pcf8574_lcd_write_nibble(lcd, 0x02, 0);
    pcf8574_lcd_send_cmd(lcd, 0x28);
    pcf8574_lcd_send_cmd(lcd, 0x0C);
    pcf8574_lcd_send_cmd(lcd, 0x06);
    pcf8574_lcd_clear(lcd);
}

void pcf8574_lcd_clear(PCF8574_LCD *lcd) {
    pcf8574_lcd_send_cmd(lcd, 0x01);
    HAL_Delay(2);
}

void pcf8574_lcd_write_nibble(PCF8574_LCD *lcd, uint8_t nibble, uint8_t rs) {
    uint8_t data = nibble << lcd->D4_BIT;
    data |= rs << lcd->RS_BIT;
    data |= lcd->backlight_state << lcd->BL_BIT; // Include backlight state in data
    data |= 1 << lcd->EN_BIT;
    HAL_I2C_Master_Transmit(lcd->hi2c, lcd->I2C_ADDR << 1, &data, 1, 100);
    HAL_Delay(1);
    data &= ~(1 << lcd->EN_BIT);
    HAL_I2C_Master_Transmit(lcd->hi2c, lcd->I2C_ADDR << 1, &data, 1, 100);
}

void pcf8574_lcd_send_cmd(PCF8574_LCD *lcd, uint8_t cmd) {
    uint8_t upper_nibble = cmd >> 4;
    uint8_t lower_nibble = cmd & 0x0F;
    pcf8574_lcd_write_nibble(lcd, upper_nibble, 0);
    pcf8574_lcd_write_nibble(lcd, lower_nibble, 0);
    if (cmd == 0x01 || cmd == 0x02) {
        HAL_Delay(2);
    }
}

void pcf8574_lcd_send_data(PCF8574_LCD *lcd, uint8_t data) {
    uint8_t upper_nibble = data >> 4;
    uint8_t lower_nibble = data & 0x0F;
    pcf8574_lcd_write_nibble(lcd, upper_nibble, 1);
    pcf8574_lcd_write_nibble(lcd, lower_nibble, 1);
}

void pcf8574_lcd_write_string(PCF8574_LCD *lcd, char *str) {
    while (*str) {
        pcf8574_lcd_send_data(lcd, *str++);
    }
}

void pcf8574_lcd_set_cursor(PCF8574_LCD *lcd, uint8_t row, uint8_t column) {
    uint8_t address;
    switch (row) {
        case 0:
            address = 0x00;
            break;
        case 1:
            address = 0x40;
            break;
        default:
            address = 0x00;
    }
    address += column;
    pcf8574_lcd_send_cmd(lcd, 0x80 | address);
}

void pcf8574_lcd_backlight(PCF8574_LCD *lcd, uint8_t state) {
    if (state) {
        lcd->backlight_state = 1;
    } else {
        lcd->backlight_state = 0;
    }
}
