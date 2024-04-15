#include "run.h"

#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "graphics.h"
#include "lcd_gfx.h"
#include "lcd_menu.h"
#include "mcp9601.h"
#include "pcf8574_lcd.h"
#include "pid.h"
#include "timeout.h"

#define PID_SCALE 5000.0f

PID_TypeDef pid_top, pid_btm;
float temperature_top, temperature_btm;
float pid_out_top, pid_out_btm;
float pid_setpoint_top, pid_setpoint_btm;

#define MCP_SLOW_WEIGHT 0.6
#define MCP_FAST_WEIGHT 0.4
#define MCP_AVG_BUF 5

#define PID_MAX_VAL ((uint32_t)(65535 / 4))
#define TEMP_ERROR_PERCENT 0.15f

MCP9601 *mcps[4] = {0};

MCP9601 **mcp_fast = mcps;
MCP9601 **mcp_slow = mcps + 2;
int mcp_fast_c = 0, mcp_slow_c = 0;

float temp_buf_top[MCP_AVG_BUF] = {0}, temp_buf_btm[MCP_AVG_BUF] = {0};
int temp_buf_top_i = 0, temp_buf_btm_i = 0;
timeout_t mcp_to_top, mcp_to_btm;

PCF8574_LCD *lcd;
LcdGFX *gfx;
LMSContext *ctx;

uint8_t enable_input = 0;
uint8_t enable_pid = 0;
uint8_t enable_temp = 0;
uint8_t enable_power = 0;

static char temp_top_str[10] = {0};
static char temp_btm_str[10] = {0};
static char pid_top_str[10] = {0};
static char pid_btm_str[10] = {0};
static char err_str[5] = {0};

int8_t setpoint_sel_0, setpoint_sel_1, setpoint_sel_2;

float calc_temp(MCP9601 *mcp_slow, MCP9601 *mcp_fast, float last_temp, timeout_t *timeout) {
    uint8_t s_fail = mcp_slow->failed || mcp_slow->status.val.open_circuit || mcp_slow->status.val.short_circuit || mcp_slow->temp.all == 0;
    uint8_t f_fail = mcp_fast->failed || mcp_fast->status.val.open_circuit || mcp_fast->status.val.short_circuit || mcp_fast->temp.all == 0;

    float last_temp_lt = last_temp - (last_temp * TEMP_ERROR_PERCENT);
    float last_temp_gt = last_temp + (last_temp * TEMP_ERROR_PERCENT);

    float fast_temp = mcp9601_temperature(mcp_fast);
    float slow_temp = mcp9601_temperature(mcp_slow);

    s_fail = s_fail && (slow_temp < last_temp_lt || slow_temp > last_temp_gt);
    f_fail = f_fail && (fast_temp < last_temp_lt || fast_temp > last_temp_gt);

    if (s_fail && f_fail) {
        TIM3->CCR3 = (((float)timeout->value) / timeout->timeout) * 65535;
        // TODO: handle worst case
        if (check_timeout(timeout)) { // After a second of failed sensors
            return 300;               // Assume everything is really hot
        } else {
            return last_temp;         // Otherwise return the last temp until the sensor fixes itself
        }
    } else if (s_fail) {
        reset_timeout(timeout);
        return fast_temp;
    } else if (f_fail) {
        reset_timeout(timeout);
        return slow_temp;
    } else {
        reset_timeout(timeout);
        return MCP_SLOW_WEIGHT * slow_temp + MCP_FAST_WEIGHT * fast_temp;
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
    temperature_top = updateRollingAverage(calc_temp(mcp_slow[1], mcp_fast[1], temperature_top, &mcp_to_top), temp_buf_top, &temp_buf_top_i);
    temperature_btm = updateRollingAverage(calc_temp(mcp_slow[0], mcp_fast[0], temperature_btm, &mcp_to_btm), temp_buf_btm, &temp_buf_btm_i);
}

void on_TIM2_update(TIM_HandleTypeDef *htim) {
    static uint32_t last = 0;
    uint32_t curr = TIM2->CNT;
    if (enable_input) {
        if (curr < last) {
            lms_signal_menu(ctx, DOWN);
        } else if (curr > last) {
            lms_signal_menu(ctx, UP);
        }
    }
    last = curr;
}

void on_TIM_17ms(TIM_HandleTypeDef *htim) {
    if (enable_pid) {
        if (PID_Compute(&pid_top)) {
            uint32_t val = pid_out_top == 0.0f ? 0 : ((pid_out_top / PID_SCALE) * ((float)PID_MAX_VAL));
            val = (val > PID_MAX_VAL) ? PID_MAX_VAL : val;
            if (enable_power) {
                TIM9->CCR1 = val;
            }
        }
        if (PID_Compute(&pid_btm)) {
            uint32_t val = pid_out_btm == 0.0f ? 0 : ((pid_out_btm / PID_SCALE) * ((float)PID_MAX_VAL));
            val = (val > PID_MAX_VAL) ? PID_MAX_VAL : val;
            if (enable_power) {
                TIM9->CCR2 = val;
            }
        }
    }
}

void on_TIM_20ms(TIM_HandleTypeDef *htim) {
    static uint8_t buffered = 0;
    if (enable_temp) {
        static int c = 0;
        mcp9601_dma_loop(mcp_fast, 2, &mcp_fast_c);     // Every 20ms
        if (c++ >= 32) {
            mcp9601_dma_loop(mcp_slow, 2, &mcp_slow_c); // Every 320ms
            c = 0;
            buffered = buffered >= 2 ? buffered : buffered + 1;
        }
        if (buffered >= 2)
            set_temperatures(); // Every 20ms
    }
}

timeout_t enter_bounce = {50}, back_bounce = {50}, cancel_bounce = {50};

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (enable_input) {
        switch (GPIO_Pin) {
            case Rotary_SW_Pin:
                if (HAL_GPIO_ReadPin(Rotary_SW_GPIO_Port, Rotary_SW_Pin) == GPIO_PIN_RESET) { // Check if button 'down'
                    reset_timeout(&enter_bounce);
                } else if (check_timeout(&enter_bounce)) {                                    // if button goes up after minimum set by timeout, button push is good.
                    lms_signal_menu(ctx, ENTER);
                }
                break;
            case Button_Back_Pin:
                if (HAL_GPIO_ReadPin(Button_Back_GPIO_Port, Button_Back_Pin) == GPIO_PIN_RESET) {
                    reset_timeout(&back_bounce);
                } else if (check_timeout(&back_bounce)) {
                    lms_signal_menu(ctx, BACK);
                }
                break;
            case Button_Cancel_Pin:
                if (HAL_GPIO_ReadPin(Button_Cancel_GPIO_Port, Button_Cancel_Pin) == GPIO_PIN_RESET) {
                    reset_timeout(&cancel_bounce);
                } else if (check_timeout(&cancel_bounce)) {
                    lms_signal_menu(ctx, CANCEL);
                }
                break;
        }
    } else {
        reset_timeout(&enter_bounce);
        reset_timeout(&back_bounce);
        reset_timeout(&cancel_bounce);
    }
}

void baseline() {
    HAL_NVIC_DisableIRQ(TIM2_IRQn);
    HAL_NVIC_DisableIRQ(TIM5_IRQn);
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
    HAL_NVIC_DisableIRQ(EXTI0_IRQn);
    HAL_NVIC_DisableIRQ(EXTI1_IRQn);
    HAL_NVIC_DisableIRQ(EXTI4_IRQn);
    enable_input = 0;
    enable_pid = 0;
    enable_temp = 0;
    enable_power = 0;
    TIM9->CCR1 = 0;
    TIM9->CCR2 = 0;
    TIM3->CCR3 = 0;
}

void root_callback_baseline(LMSPage *page) {
    baseline();
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    HAL_NVIC_EnableIRQ(TIM5_IRQn);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    enable_input = 1;
    enable_temp = 1;
}

static inline void register_callbacks() {
    HAL_TIM_RegisterCallback(&htim2, HAL_TIM_IC_CAPTURE_CB_ID, on_TIM2_update);
    HAL_TIM_RegisterCallback(&htim4, HAL_TIM_PERIOD_ELAPSED_CB_ID, on_TIM_17ms);
    HAL_TIM_RegisterCallback(&htim5, HAL_TIM_PERIOD_ELAPSED_CB_ID, on_TIM_20ms);
}

static inline void splash() {
    lcdGFX_draw_graphic(gfx, 8, 1, 4, 2, graphic_odp);
    lcdGFX_update(gfx);
    HAL_Delay(1200);
    lcdGFX_clear_graphic(gfx);
    lcdGFX_print_string(gfx, 3, 1, "HeatPress v1.0", 0);
    lcdGFX_update(gfx);
    HAL_Delay(800);
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

void page_heat_callback(LMSPage *page) {
    reset_timeout(&mcp_to_top);
    reset_timeout(&mcp_to_btm);

    enable_input = 1;
    enable_temp = 1;
    enable_pid = 1;
    enable_power = 1;
    TIM9->CCR1 = 0;
    TIM9->CCR2 = 0;
    float setpoint = (setpoint_sel_2 * 100) + (setpoint_sel_1 * 10) + setpoint_sel_0;
    pid_setpoint_top = setpoint;
    pid_setpoint_btm = setpoint;
}

static inline void setup_menu() {
    LMSPage *root = lms_new_page(ctx, "root", 20, 4, root_callback_baseline);

    LMSPage *page_set = lms_new_page(ctx, "page_set", 20, 4, NULL);
    LMSPage *page_tim = lms_new_page(ctx, "page_tim", 20, 4, NULL);
    LMSPage *page_man = lms_new_page(ctx, "page_man", 20, 4, NULL);
    LMSPage *page_heat = lms_new_page(ctx, "page_heat", 20, 4, page_heat_callback);

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

    LMSTxt *txt_set_setpoint = lms_new_text(page_set, "Set Temp Target", 0);
    LMSNumSel *numsel_hunds = lms_new_num_sel(page_set, "Hundreds Place", &setpoint_sel_2);
    LMSNumSel *numsel_tens = lms_new_num_sel(page_set, "Tens Place", &setpoint_sel_1);
    LMSNumSel *numsel_ones = lms_new_num_sel(page_set, "Ones Place", &setpoint_sel_0);
    LMSTxt *txt_set_celcius = lms_new_text(page_set, "C", 0);

    LMS_SET_POS(txt_set_setpoint, 2, 0);
    LMS_SET_POS(numsel_hunds, 8, 1);
    LMS_SET_POS(numsel_tens, 9, 1);
    LMS_SET_POS(numsel_ones, 10, 1);
    LMS_SET_POS(txt_set_celcius, 11, 2);

    LMS_SET_ALL_DIR(page_set, numsel_hunds);

    LMS_SET_BACK(numsel_ones, numsel_tens);
    LMS_SET_BACK(numsel_tens, numsel_hunds);
    LMS_SET_BACK(numsel_hunds, root);

    LMS_SET_NEXT(numsel_hunds, numsel_tens);
    LMS_SET_NEXT(numsel_tens, numsel_ones);
    LMS_SET_NEXT(numsel_ones, page_heat);

    LMSTxt *txt_top_temp = lms_new_text(page_heat, temp_top_str, 0);
    LMS_SET_POS(txt_top_temp, 0, 0);
    LMSTxt *txt_btm_temp = lms_new_text(page_heat, temp_btm_str, 0);
    LMS_SET_POS(txt_btm_temp, 7, 0);

    LMSTxt *txt_top_pid = lms_new_text(page_heat, pid_top_str, 0);
    LMS_SET_POS(txt_top_pid, 0, 1);
    LMSTxt *txt_btm_pid = lms_new_text(page_heat, pid_btm_str, 0);
    LMS_SET_POS(txt_btm_pid, 7, 1);

    LMSTxt *txt_err_str = lms_new_text(page_heat, err_str, 0);
    LMS_SET_POS(txt_err_str, 0, 2);

    LMS_SET_BACK(page_heat, root);

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

    mcp_to_top.timeout = 3000;
    mcp_to_btm.timeout = 3000;
}

static inline void setup_pid() {
    PID(&pid_top, &temperature_top, &pid_out_top, &pid_setpoint_top, 2, 5, 1, _PID_P_ON_E, _PID_CD_DIRECT);
    PID(&pid_btm, &temperature_btm, &pid_out_btm, &pid_setpoint_btm, 2, 5, 1, _PID_P_ON_E, _PID_CD_DIRECT);
    PID_SetMode(&pid_top, _PID_MODE_AUTOMATIC);
    PID_SetMode(&pid_btm, _PID_MODE_AUTOMATIC);
    PID_SetOutputLimits(&pid_top, 0, PID_SCALE);
    PID_SetOutputLimits(&pid_btm, 0, PID_SCALE);
    PID_SetSampleTime(&pid_top, 17);
    PID_SetSampleTime(&pid_btm, 17);
}

static inline void setup_timers() {
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    HAL_NVIC_EnableIRQ(TIM5_IRQn);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL);
    HAL_TIM_Base_Start_IT(&htim4);
    HAL_TIM_Base_Start_IT(&htim5);
}

void run() {

    register_callbacks();

    baseline();

    enable_temp = 1;

    setup_timers();

    setup_thermal_probes();
    setup_pid();

    lcd = pcf8574_lcd_new(&hi2c2, 0x27, 4, 20, LCD_5x8DOTS);
    gfx = lcdGFX_new(lcd);
    ctx = lms_new_menu(gfx);
    configure_menu();
    setup_menu();

    splash();
    set_sprites();

    enable_input = 1;

    timeout_t temp_to = {.timeout = 3000};
    uint32_t noti = 0, noti_last = 0;

    while (1) {
        if (enable_power) {
            sprintf(temp_top_str, "% 5.1fC", temperature_top);
            sprintf(temp_btm_str, "% 5.1fC", temperature_btm);
            sprintf(pid_top_str, "% 5.1f", pid_out_top == 0.0f ? 0 : (pid_out_top / (PID_SCALE / 100)));
            sprintf(pid_btm_str, "% 5.1f", pid_out_btm == 0.0f ? 0 : (pid_out_btm / (PID_SCALE / 100)));
            sprintf(err_str, "%c%c%c%c", mcp9601_err_str(mcps[1])[0], mcp9601_err_str(mcps[3])[0], mcp9601_err_str(mcps[0])[0], mcp9601_err_str(mcps[2])[0]);
        }

        if (noti) {
            noti_last++;
            if (noti_last > noti) {
                lcdGFX_clear_buffer(gfx);
                noti_last = 0;
                noti = 0;
            }
        }

        static timeout_t force_refresh = {10000};

        if (check_timeout(&force_refresh)) {
            // lcdGFX_clear_buffer(gfx);
            gfx->refresh = 1;
            reset_timeout(&force_refresh);
        }

        lms_update_menu(ctx);

        if (temperature_top >= 60 || temperature_btm >= 60) {
            pcf8574_lcd_set_cursor(lcd, 0, 19);
            pcf8574_lcd_send_data(lcd, 1);
            noti++;
        }

        if (TIM9->CCR1 > 0 || TIM9->CCR2 > 0) {
            pcf8574_lcd_set_cursor(lcd, 1, 19);
            pcf8574_lcd_send_data(lcd, 2);
            noti++;
        }

        if (temperature_btm > 250 || temperature_top > 250) { // After reading > 250C for more than 3 seconds the system will auto shut-off
            pcf8574_lcd_set_cursor(lcd, 2, 19);
            pcf8574_lcd_send_data(lcd, 0);
            noti++;
            if (check_timeout(&temp_to)) {
                break;
            }
        } else {
            reset_timeout(&temp_to);
        }
    }

    pcf8574_lcd_clear(lcd);
    pcf8574_lcd_set_cursor(lcd, 0, 5);
    pcf8574_lcd_write_string(lcd, "BREAK MODE");
    pcf8574_lcd_set_cursor(lcd, 1, 2);
    pcf8574_lcd_write_string(lcd, "CAUTION ON HEAT");
    pcf8574_lcd_set_cursor(lcd, 2, 1);
    pcf8574_lcd_write_string(lcd, "PLEASE POWER CYCLE");
    pcf8574_lcd_set_cursor(lcd, 3, 1);
    pcf8574_lcd_write_string(lcd, "HEAT TEMP EXCEEDED");

    while (1) {
        baseline();
    }
}
