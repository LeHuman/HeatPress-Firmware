#include "lcd_gfx.h"

#include <memory.h>
#include <stdint.h>
#include <stdlib.h>

LcdGFX *lcdGFX_new(PCF8574_LCD *lcd) {
    LcdGFX *gfx = calloc(1, sizeof(LcdGFX));

    gfx->lcd = lcd;
    gfx->buffer = calloc(lcd->characters, sizeof(int8_t));
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

// #define DEBUG_UPDATE_MASK
// #define DISABLE_UPDATE_MASK

void lcdGFX_update(LcdGFX *gfx) {
    if (HAL_GetTick() >= gfx->set_time) {
        uint16_t c = 0, nx = 0, ny = 0;
        for (size_t y = 0; y < gfx->lcd->characters / gfx->lcd->columns; y++) {
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
                if (lcdGFX_hash(c) & gfx->update) {
                    if (nx != x) {
                        pcf8574_lcd_set_cursor(gfx->lcd, y, x);
                        nx = x;
                        // ny = y;
                    }
                    pcf8574_lcd_send_data(gfx->lcd, gfx->buffer[c]);
                    nx++;
                    // ny = y;
                    // if (nx > gfx->lcd->columns) {
                    //     nx = 0;
                    // }
                    // ny = (y + ((nx == 0) * 2) - ((y == gfx->lcd->rows) * 2)) % gfx->lcd->rows; // Default behavior makes continuous writes go every other line.
                    // ny = (y + ((nx == 0) * 2)) % gfx->lcd->rows;
                }
    #endif
#endif
                c++;
            }
        }
        gfx->update = 0;
        gfx->set_time = HAL_GetTick() + gfx->refresh_ms;
    }
}

void lcdGFX_set_char_abs(LcdGFX *gfx, int i, char c) {
    if (gfx->buffer[i] != c) {
        gfx->buffer[i] = c;
        gfx->update |= lcdGFX_hash(i);
    }
}

void lcdGFX_set_char(LcdGFX *gfx, int x, int y, const char c) {
    int i = lcdGFX_xy2abs(gfx, x, y);
    lcdGFX_set_char_abs(gfx, i, c);
}

void lcdGFX_set_string(LcdGFX *gfx, int x, int y, char *str, int pad) {
    int i = lcdGFX_xy2abs(gfx, x, y);
    // int limit = gfx->lcd->columns - x + 1;
    for (size_t j = x; j < gfx->lcd->columns; j++) {
        if (*str)
            lcdGFX_set_char_abs(gfx, i++, *str++);
        else if (pad > 0)
            lcdGFX_set_char_abs(gfx, i++, ' ');
        else
            break;
        --pad;
    }

    // while (*str && (--limit > 0)) {
    //     lcdGFX_set_char_abs(gfx, i++, *str++);
    //     --pad;
    // }
    // while ((pad > 0) && (--limit > 0)) {
    //     lcdGFX_set_char_abs(gfx, i++, ' ');
    //     --pad;
    // }
}

// void lcdGFX_draw_line(LcdGFX *gfx, int x0, int y0, int x1, int y1, const char c) {

// }