#include "run.h"

#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "pcf8574_lcd.h"

void run(I2C_HandleTypeDef *hi2c1, I2C_HandleTypeDef *hi2c2, I2C_HandleTypeDef *hi2c3) {
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, 1);
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 1);
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, 1);

    HAL_GPIO_WritePin(PR_CTRL_0_GPIO_Port, PR_CTRL_0_Pin, 0);
    HAL_GPIO_WritePin(PR_CTRL_1_GPIO_Port, PR_CTRL_1_Pin, 0);

    // uint8_t buffer[32] = {0};

    PCF8574_LCD *lcd = pcf8574_lcd_new_min(hi2c2, 0x27, 4, 16);

    pcf8574_lcd_initialize(lcd);
    pcf8574_lcd_backlight(lcd, 1);

    char *text = "EmbeddedThere";
    char int_to_str[10];
    int count = 0;

    pcf8574_lcd_clear(lcd);
    pcf8574_lcd_set_cursor(lcd, 0, 0);
    pcf8574_lcd_write_string(lcd, text);

    while (1) {
        sprintf(int_to_str, "%d", count);

        pcf8574_lcd_set_cursor(lcd, 1, 0);
        pcf8574_lcd_write_string(lcd, int_to_str);
        count++;
        memset(int_to_str, 0, sizeof(int_to_str));
        HAL_Delay(1500);
    }

    // HAL_I2C_Master_Transmit(hi2c3, 0x00, &buffer, 1, 1000);
    // HAL_I2C_Master_Transmit(hi2c3, 0x27, &buffer, 1, 1000);
}
