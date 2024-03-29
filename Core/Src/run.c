#include "run.h"

#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lcd_gfx.h"
#include "lcd_menu.h"
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

// volatile int8_t direction = 0, back = 0, cancel = 0;
// volatile int8_t rotary_button_not = 0;
// volatile uint32_t rotary_push_count = 0;

uint8_t input_enabled = 0;
LMSContext *ctx;

void on_TIM3_update(TIM_HandleTypeDef *htim) {
    static uint32_t last = 0;
    uint32_t curr = TIM2->CNT;
    if (input_enabled) {
        if (curr < last) {
            lms_signal_menu(ctx, UP);
        } else if (curr > last) {
            lms_signal_menu(ctx, DOWN);
        }
    }
    last = curr;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    switch (GPIO_Pin) {
        case Rotary_SW_Pin:
            // static uint32_t last = 1;
            static uint32_t next_update = 0;

            uint32_t time = HAL_GetTick();
            // uint8_t val = HAL_GPIO_ReadPin(Rotary_SW_GPIO_Port, Rotary_SW_Pin);

            if (next_update <= time) { // || last != (val)) {
                // rotary_push_count += val;
                // rotary_button_not = val;
                // last = val;
                lms_signal_menu(ctx, ENTER);
            }
            next_update = time + 300;
            break;
        case Button_Back_Pin:
            lms_signal_menu(ctx, BACK);
            // back = HAL_GPIO_ReadPin(Button_Back_GPIO_Port, Button_Back_Pin);
            break;
        case Button_Cancel_Pin:
            lms_signal_menu(ctx, CANCEL);
            // cancel = HAL_GPIO_ReadPin(Button_Cancel_GPIO_Port, Button_Cancel_Pin);
            break;
    }
}

void run(I2C_HandleTypeDef *hi2c1, I2C_HandleTypeDef *hi2c2, I2C_HandleTypeDef *hi2c3, TIM_HandleTypeDef *htim2) {
    // HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, 1);
    // HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 1);
    // HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, 1);

    // HAL_GPIO_WritePin(PR_CTRL_0_GPIO_Port, PR_CTRL_0_Pin, 1);
    // HAL_GPIO_WritePin(PR_CTRL_1_GPIO_Port, PR_CTRL_1_Pin, 1);

    HAL_TIM_RegisterCallback(htim2, HAL_TIM_IC_CAPTURE_CB_ID, on_TIM3_update);

    PCF8574_LCD *lcd = pcf8574_lcd_new(hi2c2, 0x27, 4, 20, LCD_5x8DOTS);
    LcdGFX *gfx = lcdGFX_new(lcd);
    ctx = lms_new_menu(gfx);

    LMSPage *root = lms_new_page(ctx, "root", 20, 4, NULL);
    LMSTxt *root_title = lms_new_text(root, "Hello There!", 0);
    LMSBtn *btn = lms_new_btn(root, "test_btn", 3, 2, NULL);
    LMSNumSel *num = lms_new_num_sel(root, "test_num");

    LMS_SET_POS(root_title, 2, 0);
    LMS_SET_POS(btn, 1, 2);
    LMS_SET_POS(num, 16, 0);

    LMS_SET_NEXT(root, btn);
    LMS_SET_NEXT(btn, num);
    LMS_SET_NEXT(num, root);

    // LMS_SET_NEXT();

    lms_initialize_menu(ctx, root);

    input_enabled = 1;

    // int set_time = HAL_GetTick();

    while (1) {
        // if (HAL_GetTick() >= set_time) {
        //     set_time = HAL_GetTick() + 1000;
        //     HAL_GPIO_TogglePin(PR_CTRL_0_GPIO_Port, PR_CTRL_0_Pin);
        //     HAL_GPIO_TogglePin(PR_CTRL_1_GPIO_Port, PR_CTRL_1_Pin);
        //     HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
        // }

        lms_update_menu(ctx);
    }
}

// void run(I2C_HandleTypeDef *hi2c1, I2C_HandleTypeDef *hi2c2, I2C_HandleTypeDef *hi2c3, TIM_HandleTypeDef *htim2) {
//     // HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, 1);
//     // HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 1);
//     // HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, 1);

//     // HAL_GPIO_WritePin(PR_CTRL_0_GPIO_Port, PR_CTRL_0_Pin, 1);
//     // HAL_GPIO_WritePin(PR_CTRL_1_GPIO_Port, PR_CTRL_1_Pin, 1);

//     HAL_TIM_RegisterCallback(htim2, HAL_TIM_IC_CAPTURE_CB_ID, on_TIM3_update);

//     PCF8574_LCD *lcd = pcf8574_lcd_new(hi2c2, 0x27, 4, 20, LCD_5x8DOTS);
//     LcdGFX *gfx = lcdGFX_new(lcd);
//     lcdGFX_initialize(gfx);

//     char int_to_str[32];
//     // int count = 0;

//     // int delay = 300;
//     // int set_time = HAL_GetTick();

//     // pcf8574_lcd_fling_char(lcd, odp_0, 1, 8);
//     // pcf8574_lcd_fling_char(lcd, odp_1, 1, 9);
//     // pcf8574_lcd_fling_char(lcd, odp_2, 1, 10);
//     // pcf8574_lcd_fling_char(lcd, odp_3, 1, 11);
//     // pcf8574_lcd_fling_char(lcd, odp_4, 2, 8);
//     // pcf8574_lcd_fling_char(lcd, odp_5, 2, 9);
//     // pcf8574_lcd_fling_char(lcd, odp_6, 2, 10);
//     // pcf8574_lcd_fling_char(lcd, odp_7, 2, 11);
//     // HAL_Delay(1200);
//     // pcf8574_lcd_clear(lcd);
//     // pcf8574_lcd_set_cursor(lcd, 1, 3);
//     // pcf8574_lcd_write_string(lcd, "HeatPress v1.0");
//     // pcf8574_lcd_set_cursor(lcd, 0, 0);
//     // HAL_Delay(1000);
//     // pcf8574_lcd_clear(lcd);

//     while (1) {
//         // if (HAL_GetTick() >= set_time) {
//         // count += 111;
//         // count %= 1000;

//         sprintf(int_to_str, "%d", rotary_push_count);
//         lcdGFX_print_string(gfx, 0, 2, int_to_str, 5);
//         lcdGFX_print_string(gfx, 0, 0, direction == 0 ? "still" : (direction > 0 ? "up" : "down"), 5);
//         lcdGFX_print_string(gfx, 10, 0, back ? "no back" : "back", 7);
//         lcdGFX_print_string(gfx, 10, 1, cancel ? "no cancel" : "cancel", 9);
//         lcdGFX_print_string(gfx, 0, 1, rotary_button_not ? "not pushed" : "pushed", 8);
//         // lcdGFX_print_string(gfx, rndInt(0, 19), rndInt(0, 3), int_to_str, 4);
//         // lcdGFX_draw_line(gfx, 0, 0, 19, 3, 255);
//         // set_time = HAL_GetTick() + delay;
//         // }

//         lcdGFX_update(gfx);
//     }
// }
