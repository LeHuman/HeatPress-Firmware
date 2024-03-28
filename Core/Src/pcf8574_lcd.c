#include "pcf8574_lcd.h"

#include <stdint.h>
#include <stdlib.h>

#include "pcf8574_address.h"

inline static void delay_ms(int ms) {
    HAL_Delay(ms);
}

inline static int i2c_transmit(PCF8574_LCD *lcd, uint8_t *data, uint16_t size, uint32_t timeout) {
    return HAL_I2C_Master_Transmit(lcd->hi2c, lcd->address << 1, data, size, timeout);
    // return HAL_I2C_Master_Transmit_IT(lcd->hi2c, lcd->address << 1, data, size);
}

PCF8574_LCD *pcf8574_lcd_new(I2C_HandleTypeDef *hi2c, uint8_t address, uint8_t rows, uint8_t columns, PCF8574_lcd_format char_format) {
    PCF8574_LCD *lcd = calloc(1, sizeof(PCF8574_LCD));
    if (lcd != NULL) {
        lcd->hi2c = hi2c;
        lcd->address = address;

        // Set default # lines, font size, etc.
        lcd->display.function = LCD_4BITMODE | char_format;
        // Turn the display on with no cursor or blinking default
        lcd->display.control = CTRL_DISPLAYON | CTRL_CURSOROFF | CTRL_BLINKOFF;
        // Initialize to default text direction (for roman languages)
        lcd->display.mode = DISPLAY_ENTRYLEFT | DISPLAY_ENTRYSHIFTDECREMENT;

        if (rows > 1) {
            lcd->display.function |= LCD_2LINE;
        } else {
            lcd->display.function |= LCD_1LINE;
        }

        lcd->char_format = char_format;
        lcd->backlight = 1;

        lcd->columns = columns;
        lcd->rows = rows;

        lcd->characters = columns * rows;
    }

    return lcd;
}

PCF8574_LCD *pcf8574_lcd_new_default(I2C_HandleTypeDef *hi2c) {
    return pcf8574_lcd_new(hi2c, 0x27, 2, 16, CHAR_5x8DOTS);
}

void pcf8574_lcd_free(PCF8574_LCD *lcd) {
    free(lcd);
}

void pcf8574_lcd_initialize(PCF8574_LCD *lcd) {
    // Set to 4-bit interface
    delay_ms(50);
    pcf8574_lcd_write_nibble(lcd, 0x03, 0);
    delay_ms(5);
    pcf8574_lcd_write_nibble(lcd, 0x03, 0);
    delay_ms(1);
    pcf8574_lcd_write_nibble(lcd, 0x03, 0);
    delay_ms(1);
    pcf8574_lcd_write_nibble(lcd, 0x02, 0);

    pcf8574_lcd_send_cmd(lcd, CMD_FUNCTIONSET | lcd->display.function);
    pcf8574_lcd_send_cmd(lcd, CMD_DISPLAYCONTROL | lcd->display.control);
    pcf8574_lcd_send_cmd(lcd, CMD_ENTRYMODESET | lcd->display.mode);

    pcf8574_lcd_clear(lcd);
}

void pcf8574_lcd_clear(PCF8574_LCD *lcd) {
    pcf8574_lcd_send_cmd(lcd, CMD_CLEARDISPLAY);
    delay_ms(2);
}

void pcf8574_lcd_write_nibble(PCF8574_LCD *lcd, uint8_t nibble, uint8_t rs) {
    uint8_t data = nibble << 4;
    data |= rs * BIT_RS;
    data |= lcd->backlight * BIT_BL; // Include backlight state in data
    data |= BIT_EN;
    i2c_transmit(lcd, &data, 1, 100);
    delay_ms(1);
    data &= ~BIT_EN;
    i2c_transmit(lcd, &data, 1, 100);
}

void pcf8574_lcd_send_cmd(PCF8574_LCD *lcd, uint8_t cmd) {
    uint8_t upper_nibble = cmd >> 4;
    uint8_t lower_nibble = cmd & 0x0F;
    pcf8574_lcd_write_nibble(lcd, upper_nibble, 0);
    pcf8574_lcd_write_nibble(lcd, lower_nibble, 0);
    if (cmd == CMD_CLEARDISPLAY || cmd == CMD_RETURNHOME) {
        delay_ms(2);
    }
}

void pcf8574_lcd_send_data(PCF8574_LCD *lcd, uint8_t data) {
    uint8_t upper_nibble = (data >> 4);
    uint8_t lower_nibble = (data & 0x0F);
    pcf8574_lcd_write_nibble(lcd, upper_nibble, 1);
    pcf8574_lcd_write_nibble(lcd, lower_nibble, 1);
}

void pcf8574_lcd_write_string(PCF8574_LCD *lcd, char *str) {
    while (*str) {
        pcf8574_lcd_send_data(lcd, *str++);
    }
}

void pcf8574_lcd_set_cursor(PCF8574_LCD *lcd, uint8_t row, uint8_t column) {
    static int row_offsets[] = {0x00, 0x40, 0x14, 0x54};

    if (row > lcd->rows) {
        row = lcd->rows - 1;
    }

    pcf8574_lcd_send_cmd(lcd, CMD_SETDDRAMADDR | (column + row_offsets[row]));
}

void pcf8574_lcd_backlight(PCF8574_LCD *lcd, uint8_t state) {
    lcd->backlight = !!state;
}

void pcf8574_lcd_create_char(PCF8574_LCD *lcd, uint8_t location, uint8_t charmap[8]) {
    location &= 0x7; // We only have 8 locations 0-7
    pcf8574_lcd_send_cmd(lcd, CMD_SETCGRAMADDR | (location << 3));
    for (int i = 0; i < 8; i++) {
        pcf8574_lcd_send_data(lcd, charmap[i]);
    }
}

void pcf8574_lcd_fling_char(PCF8574_LCD *lcd, uint8_t charmap[8], uint8_t row, uint8_t column) {
    static uint8_t i = 0;
    pcf8574_lcd_create_char(lcd, i, charmap);
    pcf8574_lcd_set_cursor(lcd, row, column);
    pcf8574_lcd_send_data(lcd, i++);
}