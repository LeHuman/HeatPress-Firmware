#include "run.h"

#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "graphics.h"
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
            lms_signal_menu(ctx, DOWN);
            // lcdGFX_print_char(ctx->gfx, 19, 0, 'v');
        } else if (curr > last) {
            lms_signal_menu(ctx, UP);
            // lcdGFX_print_char(ctx->gfx, 19, 0, '^');
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
            // lcdGFX_print_char(ctx->gfx, 19, 0, 'E');
            break;
        case Button_Back_Pin:
            lms_signal_menu(ctx, BACK);
            // back = HAL_GPIO_ReadPin(Button_Back_GPIO_Port, Button_Back_Pin);
            // lcdGFX_print_char(ctx->gfx, 19, 0, 'B');
            break;
        case Button_Cancel_Pin:
            // lcdGFX_print_char(ctx->gfx, 19, 0, 'C');
            lms_signal_menu(ctx, CANCEL);
            // cancel = HAL_GPIO_ReadPin(Button_Cancel_GPIO_Port, Button_Cancel_Pin);
            break;
    }
}

// TIM_HandleTypeDef *tim3;

void baseline() {
    TIM9->CCR1 = 0;
    TIM9->CCR2 = 0;
    TIM3->CCR3 = (!TIM3->CCR3) * 65535;
}

void root_callback_baseline(LMSPage *page) {
    baseline();
}

void run(I2C_HandleTypeDef *hi2c1, I2C_HandleTypeDef *hi2c2, I2C_HandleTypeDef *hi2c3, TIM_HandleTypeDef *htim2, TIM_HandleTypeDef *htim3) {
    // HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, 1);
    // HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 1);
    // HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, 1);

    // HAL_GPIO_WritePin(PR_CTRL_0_GPIO_Port, PR_CTRL_0_Pin, 1);
    // HAL_GPIO_WritePin(PR_CTRL_1_GPIO_Port, PR_CTRL_1_Pin, 1);

    baseline();

    HAL_TIM_RegisterCallback(htim2, HAL_TIM_IC_CAPTURE_CB_ID, on_TIM3_update);

    PCF8574_LCD *lcd = pcf8574_lcd_new(hi2c2, 0x27, 4, 20, LCD_5x8DOTS);
    LcdGFX *gfx = lcdGFX_new(lcd);

    lcdGFX_store_sprite(gfx, sprite_alert, 0);
    lcdGFX_store_sprite(gfx, sprite_hot, 1);
    lcdGFX_store_sprite(gfx, sprite_voltage, 2);
    lcdGFX_store_anim_sprite(gfx, sprite_anim_up, 3);
    lcdGFX_store_anim_sprite(gfx, sprite_anim_down, 4);
    lcdGFX_store_anim_sprite(gfx, sprite_anim_selected, 5);
    lcdGFX_store_sprite(gfx, sprite_unselected, 6);
    lcdGFX_store_anim_sprite(gfx, sprite_anim_loading, 7);

    ctx = lms_new_menu(gfx);

    ctx->default_chars.btn.focus = 5;
    ctx->default_chars.btn.nofocus = 6;

    ctx->default_chars.num.up.focus = 3;
    ctx->default_chars.num.down.focus = 4;
    ctx->default_chars.num.up.nofocus = '^';
    ctx->default_chars.num.down.nofocus = 'v';

    LMSPage *root = lms_new_page(ctx, "root", 20, 4, root_callback_baseline);
    LMSPage *page_set = lms_new_page(ctx, "page_set", 20, 4, NULL);
    LMSPage *page_tim = lms_new_page(ctx, "page_tim", 20, 4, NULL);
    LMSPage *page_man = lms_new_page(ctx, "page_man", 20, 4, NULL);

    LMSTxt *root_title = lms_new_text(root, "Select Mode", 0);

    LMSTxt *txt_setpoint = lms_new_text(root, "Heat by Setpoint", 0);
    LMSTxt *txt_timer = lms_new_text(root, "Heat by Timer", 0);
    LMSTxt *txt_manual = lms_new_text(root, "Heat Manually", 0);

    LMSBtn *btn_mode_setpoint = lms_new_btn(root, "btn_mode_setpoint", 1, 1, NULL);
    LMSBtn *btn_mode_timer = lms_new_btn(root, "btn_mode_timer", 1, 1, NULL);
    LMSBtn *btn_mode_manual = lms_new_btn(root, "btn_mode_manual", 1, 1, NULL);

    LMS_SET_POS(root_title, 4, 0);

    LMS_SET_POS(btn_mode_setpoint, 0, 1);
    LMS_SET_POS(btn_mode_timer, 0, 2);
    LMS_SET_POS(btn_mode_manual, 0, 3);

    LMS_SET_POS(txt_setpoint, 2, 1);
    LMS_SET_POS(txt_timer, 2, 2);
    LMS_SET_POS(txt_manual, 2, 3);

    LMS_SET_ALL_DIR(root, btn_mode_setpoint);

    LMS_SET_DOWN(btn_mode_setpoint, btn_mode_timer);
    LMS_SET_DOWN(btn_mode_timer, btn_mode_manual);
    LMS_SET_DOWN(btn_mode_manual, btn_mode_setpoint);

    LMS_SET_UP(btn_mode_setpoint, btn_mode_manual);
    LMS_SET_UP(btn_mode_manual, btn_mode_timer);
    LMS_SET_UP(btn_mode_timer, btn_mode_setpoint);

    LMS_SET_NEXT(btn_mode_setpoint, page_set);
    LMS_SET_NEXT(btn_mode_manual, page_tim);
    LMS_SET_NEXT(btn_mode_timer, page_man);

    lms_initialize_menu(ctx, root);
    input_enabled = 1;

    lcdGFX_draw_graphic(gfx, 8, 1, 4, 2, graphic_odp);
    lcdGFX_update(gfx);
    HAL_Delay(700);
    lcdGFX_clear_graphic(gfx);
    lcdGFX_print_string(gfx, 3, 1, "HeatPress v1.0", 0);
    lcdGFX_update(gfx);
    HAL_Delay(700);
    lcdGFX_clear(gfx);
    lcdGFX_update(gfx);

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