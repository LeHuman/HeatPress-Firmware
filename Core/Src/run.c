#include "run.h"

#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lcd_gfx.h"
#include "pcf8574_lcd.h"

// Define LCG parameters
#define RAND_A 1103515245
#define RAND_C 12345
#define RAND_M 0x80000000 // 2^31

// Initialize the seed value (can be any non-zero value)
static uint32_t rand_seed = 1;

// Generate a random integer within the given range [min, max]
int32_t rndInt(int32_t min, int32_t max) {
    // Ensure min <= max
    if (min > max) {
        int32_t temp = min;
        min = max;
        max = temp;
    }

    // Generate a pseudo-random number using LCG algorithm
    rand_seed = (RAND_A * rand_seed + RAND_C) % RAND_M;

    // Scale and shift the random number to fit within the range [min, max]
    return (int32_t)(((uint64_t)rand_seed * (max - min + 1)) >> 31) + min;
}

uint8_t odp_0[] = {0x00, 0x00, 0x00, 0x07, 0x08, 0x10, 0x10, 0x10};
uint8_t odp_1[] = {0x00, 0x00, 0x00, 0x03, 0x02, 0x12, 0x12, 0x12};
uint8_t odp_2[] = {0x00, 0x00, 0x00, 0x10, 0x08, 0x04, 0x04, 0x04};
uint8_t odp_3[] = {0x00, 0x00, 0x00, 0x1e, 0x11, 0x11, 0x11, 0x1e};
uint8_t odp_4[] = {0x00, 0x10, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00};
uint8_t odp_5[] = {0x00, 0x12, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00};
uint8_t odp_6[] = {0x00, 0x04, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00};
uint8_t odp_7[] = {0x00, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00};

void run(I2C_HandleTypeDef *hi2c1, I2C_HandleTypeDef *hi2c2, I2C_HandleTypeDef *hi2c3) {
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, 1);
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 1);
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, 1);

    HAL_GPIO_WritePin(PR_CTRL_0_GPIO_Port, PR_CTRL_0_Pin, 0);
    HAL_GPIO_WritePin(PR_CTRL_1_GPIO_Port, PR_CTRL_1_Pin, 0);

    // uint8_t buffer[32] = {0};

    PCF8574_LCD *lcd = pcf8574_lcd_new(hi2c2, 0x27, 4, 20, LCD_5x8DOTS);
    LcdGFX *gfx = lcdGFX_new(lcd);
    lcdGFX_initialize(gfx);

    // char *text = "EmbeddedThere";
    char int_to_str[32];
    int count = 0, c = 0;

    // pcf8574_lcd_set_cursor(lcd, 0, 0);
    // pcf8574_lcd_write_string(lcd, text);

    int delay = 300;
    int set_time = HAL_GetTick();

    pcf8574_lcd_fling_char(lcd, odp_0, 1, 8);
    pcf8574_lcd_fling_char(lcd, odp_1, 1, 9);
    pcf8574_lcd_fling_char(lcd, odp_2, 1, 10);
    pcf8574_lcd_fling_char(lcd, odp_3, 1, 11);
    pcf8574_lcd_fling_char(lcd, odp_4, 2, 8);
    pcf8574_lcd_fling_char(lcd, odp_5, 2, 9);
    pcf8574_lcd_fling_char(lcd, odp_6, 2, 10);
    pcf8574_lcd_fling_char(lcd, odp_7, 2, 11);
    HAL_Delay(1200);
    pcf8574_lcd_clear(lcd);
    pcf8574_lcd_set_cursor(lcd, 1, 3);
    pcf8574_lcd_write_string(lcd, "HeatPress v1.0");
    pcf8574_lcd_set_cursor(lcd, 0, 0);
    HAL_Delay(1000);
    pcf8574_lcd_clear(lcd);

    while (1) {
        // pcf8574_lcd_send_data(gfx->lcd, 'x');

        if (HAL_GetTick() >= set_time) {

            count += 111;
            // ++count;
            count %= 1000;

            sprintf(int_to_str, "%d", count);
            // lcdGFX_set_string(gfx, rndInt(0, 19), rndInt(0, 3), int_to_str, 4);
            // lcdGFX_set_string(gfx, c % lcd->columns, c / lcd->columns, int_to_str, 4);
            // lcdGFX_set_char(gfx, c % lcd->columns, c / lcd->columns, 'A' + (c / lcd->columns));
            // lcdGFX_update(gfx);
            pcf8574_lcd_send_data(lcd, c);
            c++;
            // c %= 80;
            // pcf8574_lcd_set_cursor(lcd, 1, 0);
            // pcf8574_lcd_write_string(lcd, int_to_str);
            // sprintf(int_to_str, "%d", delay);
            // pcf8574_lcd_set_cursor(lcd, 3, 0);
            // pcf8574_lcd_write_string(lcd, int_to_str);

            // pcf8574_lcd_send_cmd(lcd, CMD_CURSORSHIFT | SHIFT_DISPLAYMOVE | SHIFT_MOVERIGHT);

            set_time = HAL_GetTick() + delay;
        }

        // memset(int_to_str, 0, sizeof(int_to_str));
        // HAL_Delay(delay / 4);
        lcdGFX_update(gfx);
    }

    // HAL_I2C_Master_Transmit(hi2c3, 0x00, &buffer, 1, 1000);
    // HAL_I2C_Master_Transmit(hi2c3, 0x27, &buffer, 1, 1000);
}
