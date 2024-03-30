#include "run.h"

#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "graphics.h"
#include "lcd_gfx.h"
#include "lcd_menu.h"
#include "mcp9601.h"
#include "pcf8574_lcd.h"
#include "pid.h"

PID_TypeDef pid_top, pid_btm;
float temperature_top, temperature_btm;
float pid_out_top, pid_out_btm;
float pid_setpoint_top, pid_setpoint_btm;

#define MCP_SLOW_WEIGHT 0.7
#define MCP_FAST_WEIGHT 0.3
#define MCP_AVG_BUF 5

MCP9601 *mcps[4] = {0};

MCP9601 **mcp_fast = mcps;
MCP9601 **mcp_slow = mcps + 2;
int mcp_fast_c = 0, mcp_slow_c = 0;

float temp_buf_top[MCP_AVG_BUF] = {0}, temp_buf_btm[MCP_AVG_BUF] = {0};
int temp_buf_top_i = 0, temp_buf_btm_i = 0;

PCF8574_LCD *lcd;
LcdGFX *gfx;
LMSContext *ctx;

uint8_t input_enabled = 0;

float calc_temp(MCP9601 *mcp_slow, MCP9601 *mcp_fast) {
    if (mcp_slow->failed) {
        return mcp9601_temperature(mcp_fast);
    } else if (mcp_slow->failed) {
        return mcp9601_temperature(mcp_slow);
    } else {
        return MCP_SLOW_WEIGHT * mcp9601_temperature(mcp_slow) + MCP_FAST_WEIGHT * mcp9601_temperature(mcp_fast);
    }
}

int compareFloats(const void *a, const void *b) {
    float fa = *((float *)a);
    float fb = *((float *)b);
    return (fa > fb) - (fa < fb);
}

float median(float arr[], int n) {
    qsort(arr, n, sizeof(float), compareFloats);

    if (n % 2 == 0) {
        return (arr[n / 2 - 1] + arr[n / 2]) / 2;
    } else {
        return arr[n / 2];
    }
}

float updateRollingAverage(float new_temp, float temp_buf[MCP_AVG_BUF], int *counter) {
    temp_buf[*counter] = new_temp;
    *counter = (*counter + 1) % MCP_AVG_BUF;

    float rollingAverage = 0;
    for (int i = 0; i < MCP_AVG_BUF; i++) {
        rollingAverage += temp_buf[i];
    }
    rollingAverage /= MCP_AVG_BUF;

    return median(temp_buf, MCP_AVG_BUF);
    ;
}

void set_temperatures() {
    temperature_top = updateRollingAverage(calc_temp(mcp_slow[1], mcp_fast[1]), temp_buf_top, &temp_buf_top_i);
    temperature_btm = updateRollingAverage(calc_temp(mcp_slow[0], mcp_fast[0]), temp_buf_btm, &temp_buf_btm_i);
}

void on_TIM2_update(TIM_HandleTypeDef *htim) {
    static uint32_t last = 0;
    uint32_t curr = TIM2->CNT;
    if (input_enabled) {
        if (curr < last) {
            lms_signal_menu(ctx, DOWN);
        } else if (curr > last) {
            lms_signal_menu(ctx, UP);
        }
    }
    last = curr;
}

void on_TIM_17ms(TIM_HandleTypeDef *htim) {
    if (PID_Compute(&pid_top)) {
        // TIM9->CCR1 = pid_out_top;
        TIM3->CCR3 = (uint32_t)pid_out_top;
    }
    if (PID_Compute(&pid_btm)) {
        // TIM9->CCR2 = pid_out_btm;
        TIM3->CCR3 = (uint32_t)pid_out_btm;
    }
}

void on_TIM_20ms(TIM_HandleTypeDef *htim) {
    static int c = 0;
    mcp9601_dma_loop(mcp_fast, 2, &mcp_fast_c);     // Every 20ms
    if (c++ >= 32) {
        mcp9601_dma_loop(mcp_slow, 2, &mcp_slow_c); // Every 320ms
        c = 0;
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    on_TIM_17ms(NULL);
    on_TIM_20ms(NULL);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    switch (GPIO_Pin) {
        case Rotary_SW_Pin:
            static uint32_t next_update = 0;

            uint32_t time = HAL_GetTick();
            if (next_update <= time) {
                lms_signal_menu(ctx, ENTER);
            }
            next_update = time + 300;
            break;
        case Button_Back_Pin:
            lms_signal_menu(ctx, BACK);
            break;
        case Button_Cancel_Pin:
            lms_signal_menu(ctx, CANCEL);
            break;
    }
}

void baseline() {
    TIM9->CCR1 = 0;
    TIM9->CCR2 = 0;
    TIM3->CCR3 = (!TIM3->CCR3) * 65535;
}

void root_callback_baseline(LMSPage *page) {
    baseline();
}

static inline void register_callbacks() {
    HAL_TIM_RegisterCallback(&htim2, HAL_TIM_IC_CAPTURE_CB_ID, on_TIM2_update);
    HAL_TIM_RegisterCallback(&htim4, HAL_TIM_PERIOD_ELAPSED_CB_ID, on_TIM_17ms);
    HAL_TIM_RegisterCallback(&htim5, HAL_TIM_PERIOD_ELAPSED_CB_ID, on_TIM_20ms);
}

static inline void splash() {
    lcdGFX_draw_graphic(gfx, 8, 1, 4, 2, graphic_odp);
    lcdGFX_update(gfx);
    HAL_Delay(100);
    lcdGFX_clear_graphic(gfx);
    lcdGFX_print_string(gfx, 3, 1, "HeatPress v1.0", 0);
    lcdGFX_update(gfx);
    HAL_Delay(100);
    lcdGFX_clear(gfx);
    lcdGFX_update(gfx);
}

static inline void set_sprites() {
    lcdGFX_store_sprite(gfx, sprite_alert, 0);
    lcdGFX_store_sprite(gfx, sprite_hot, 1);
    lcdGFX_store_sprite(gfx, sprite_voltage, 2);
    lcdGFX_store_anim_sprite(gfx, sprite_anim_up, 3);
    lcdGFX_store_anim_sprite(gfx, sprite_anim_down, 4);
    lcdGFX_store_anim_sprite(gfx, sprite_anim_selected, 5);
    lcdGFX_store_sprite(gfx, sprite_unselected, 6);
    // lcdGFX_store_sprite(gfx, sprite_celsius, 7);
    lcdGFX_store_anim_sprite(gfx, sprite_anim_loading, 7);
}

static inline void configure_menu() {
    ctx->default_chars.btn.focus = 5;
    ctx->default_chars.btn.nofocus = 6;

    ctx->default_chars.num.up.focus = 3;
    ctx->default_chars.num.down.focus = 4;
    ctx->default_chars.num.up.nofocus = '^';
    ctx->default_chars.num.down.nofocus = 'v';
}

static inline void setup_menu() {
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
}

static inline void setup_thermal_probes() {
    MCP9601 *bottom_0 = mcp9601_new_common(&hi2c3, "Bottom Temp Fast", 0b1100000 << 1, adc_res_14, amb_res_0_25, filter_2, type_k);
    MCP9601 *top_0 = mcp9601_new_common(&hi2c1, "Top Temp Fast", 0b1100000 << 1, adc_res_14, amb_res_0_25, filter_2, type_k);
    MCP9601 *bottom_1 = mcp9601_new_common(&hi2c3, "Bottom Temp Slow", 0b1100111 << 1, adc_res_18, amb_res_0_0625, filter_4, type_k);
    MCP9601 *top_1 = mcp9601_new_common(&hi2c1, "Top Temp Slow", 0b1100111 << 1, adc_res_18, amb_res_0_0625, filter_4, type_k);

    mcp9601_init(bottom_0);
    mcp9601_init(top_0);
    mcp9601_init(bottom_1);
    mcp9601_init(top_1);

    mcps[0] = bottom_0;
    mcps[1] = top_0;
    mcps[2] = bottom_1;
    mcps[3] = top_1;
}

static inline void setup_pid() {
    PID(&pid_top, &temperature_top, &pid_out_top, &pid_setpoint_top, 2, 5, 1, _PID_P_ON_E, _PID_CD_DIRECT);
    PID(&pid_btm, &temperature_btm, &pid_out_btm, &pid_setpoint_btm, 2, 5, 1, _PID_P_ON_E, _PID_CD_DIRECT);
    PID_SetMode(&pid_top, _PID_MODE_AUTOMATIC);
    PID_SetMode(&pid_btm, _PID_MODE_AUTOMATIC);
    PID_SetOutputLimits(&pid_top, 0, 65535);
    PID_SetOutputLimits(&pid_btm, 0, 65535);
    PID_SetSampleTime(&pid_top, 17);
    PID_SetSampleTime(&pid_btm, 17);
}

pid_setpoint_top = 180;
pid_setpoint_btm = 180;

void run() {

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL);
    HAL_TIM_Base_Start_IT(&htim4);
    HAL_TIM_Base_Start_IT(&htim5);

    baseline();

    setup_thermal_probes();
    setup_pid();

    register_callbacks();

    lcd = pcf8574_lcd_new(&hi2c2, 0x27, 4, 20, LCD_5x8DOTS);
    gfx = lcdGFX_new(lcd);
    ctx = lms_new_menu(gfx);
    configure_menu();
    setup_menu();

    splash();
    set_sprites();

    input_enabled = 1;

    while (1) {
        static char buf[32];

        set_temperatures();

        sprintf(buf, "% 5.1fC %c%c", temperature_top, mcp9601_err_str(mcps[0])[0], mcp9601_err_str(mcps[2])[0]);
        lcdGFX_print_string(gfx, 0, 0, buf, 0);
        sprintf(buf, "% 5.1fC %c%c", temperature_btm, mcp9601_err_str(mcps[1])[0], mcp9601_err_str(mcps[3])[0]);
        lcdGFX_print_string(gfx, 0, 1, buf, 0);

        sprintf(buf, "%f", pid_out_top);
        lcdGFX_print_string(gfx, 12, 0, buf, 4);
        sprintf(buf, "%f", pid_out_btm);
        lcdGFX_print_string(gfx, 12, 1, buf, 4);

        lcdGFX_update(gfx);
        // lms_update_menu(ctx);
    }
}
