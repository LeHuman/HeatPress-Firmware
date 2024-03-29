#include "lcd_gfx.h"

#include <memory.h>
#include <stdint.h>
#include <stdlib.h>

LcdGFX *lcdGFX_new(PCF8574_LCD *lcd) {
    LcdGFX *gfx = calloc(1, sizeof(LcdGFX));

    gfx->lcd = lcd;
    gfx->buffer = calloc(lcd->characters, sizeof(int8_t));
    gfx->buffer_ms = calloc(lcd->characters, sizeof(uint16_t));
    gfx->update = 0;
    gfx->refresh_ms = 610;
    gfx->set_time = 0;

    return gfx;
}

void lcdGFX_free(LcdGFX *gfx) {
    free(gfx);
}

static inline UPDATE_MASK_TYPE lcdGFX_hash(int i) {
    return 1 << (i % (sizeof(UPDATE_MASK_TYPE) * 8));
}

static inline int lcdGFX_xy2abs(LcdGFX *gfx, const int x, const int y) {
    return x + (y * gfx->lcd->columns);
}

void lcdGFX_initialize(LcdGFX *gfx) {
    pcf8574_lcd_initialize(gfx->lcd);
    lcdGFX_clear(gfx);
}

void lcdGFX_clear(LcdGFX *gfx) {
    pcf8574_lcd_clear(gfx->lcd);
    memset(gfx->buffer, ' ', gfx->lcd->characters);
}

#define DISABLE_UPDATE_MASK
#define DISABLE_UPDATE_SEQ
// #define DEBUG_UPDATE_MASK

void lcdGFX_update(LcdGFX *gfx) {
    // if (HAL_GetTick() >= gfx->set_time) {
    uint16_t c = 0;
#ifndef DISABLE_UPDATE_SEQ
    uint16_t nx = 0;
#endif
    for (size_t y = 0; y < gfx->lcd->characters / gfx->lcd->columns; y++) {
        uint16_t ms = HAL_GetTick() % 0xFFFF;
        for (size_t x = 0; x < gfx->lcd->columns; x++) {
#ifdef DISABLE_UPDATE_MASK
            pcf8574_lcd_set_cursor(gfx->lcd, y, x);
            pcf8574_lcd_send_data(gfx->lcd, gfx->buffer[c]);
#else
    #ifdef DEBUG_UPDATE_MASK
            static uint8_t d_x = '0';
            if (gfx->buffer[c] && (lcdGFX_hash(c) & gfx->update)) {
                if (nx != x || ny != y)
                    pcf8574_lcd_set_cursor(gfx->lcd, y, x);
                pcf8574_lcd_send_data(gfx->lcd, d_x++);
                d_x %= ':';
                d_x = d_x ? d_x : '0';
                nx = (x + 1) % gfx->lcd->columns;
                ny = (y + (nx == 0)) % gfx->lcd->rows;
            }
    #else
            if ((lcdGFX_hash(c) & gfx->update) && (gfx->buffer_ms[c] <= ms)) {
        #ifndef DISABLE_UPDATE_SEQ
                if (nx != x) {
        #endif
                    pcf8574_lcd_set_cursor(gfx->lcd, y, x);
        #ifndef DISABLE_UPDATE_SEQ
                    nx = x;
                }
        #endif
                pcf8574_lcd_send_data(gfx->lcd, gfx->buffer[c]);
        #ifndef DISABLE_UPDATE_SEQ
                nx++;
        #endif
                gfx->buffer_ms[c] = ms + gfx->refresh_ms;
            }
    #endif
#endif
            c++;
        }
    }
    gfx->update = 0;
    // gfx->set_time = HAL_GetTick() + gfx->refresh_ms;
    // }
}

void lcdGFX_set_char_abs(LcdGFX *gfx, int i, char c) {
    if ((i >= 0) && (i < gfx->lcd->characters) && (gfx->buffer[i] != c)) {
        gfx->buffer[i] = c;
        gfx->update |= lcdGFX_hash(i);
    }
}

void lcdGFX_print_char(LcdGFX *gfx, int x, int y, const char c) {
    int i = lcdGFX_xy2abs(gfx, x, y);
    lcdGFX_set_char_abs(gfx, i, c);
}

void lcdGFX_print_string(LcdGFX *gfx, int x, int y, const char *str, int pad) {
    int i = lcdGFX_xy2abs(gfx, x, y);
    if (i < 0 || i >= gfx->lcd->characters)
        return;
    for (size_t j = x; j < gfx->lcd->columns; j++) {
        if (*str)
            lcdGFX_set_char_abs(gfx, i++, *str++);
        else if (pad > 0)
            lcdGFX_set_char_abs(gfx, i++, ' ');
        else
            break;
        --pad;
    }
}

// TODO: Menu graphics using the extra 8 chars. Where special one off graphics can be displayed disabling the menu graphics, using all 8 chars

void lcdGFX_draw_line(LcdGFX *gfx, int8_t x0, int8_t y0, int8_t x1, int8_t y1, const char c) {
    uint8_t max = gfx->lcd->columns;

    float dx = (abs(x0 - x1) * (x0 > x1 ? -1 : 1)) * 1.0f / max;
    float dy = (abs(y0 - y1) * (y0 > y1 ? -1 : 1)) * 1.0f / max;
    float x = x0, y = y0;

    for (uint8_t i = 0; i < max; i++) {
        lcdGFX_set_char_abs(gfx, lcdGFX_xy2abs(gfx, (x += dx), (y += dy)), c);
    }
}

void lcdGFX_draw_box(LcdGFX *gfx, int8_t x, int8_t y, int8_t w, int8_t h, const char c) {
    int8_t y1 = y + h, x1 = x + w;
    for (; y < y1; y++) {
        for (x = x1 - w; x < x1; x++) {
            lcdGFX_set_char_abs(gfx, lcdGFX_xy2abs(gfx, x, y), c);
        }
    }
}
