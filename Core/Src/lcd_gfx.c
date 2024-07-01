#include "lcd_gfx.h"

#include <memory.h>
#include <stdint.h>
#include <stdlib.h>

#include "graphics.h"

LcdGFX *lcdGFX_new(PCF8574_LCD *lcd) {
    LcdGFX *gfx = calloc(1, sizeof(LcdGFX));

    gfx->lcd = lcd;
    gfx->buffer = calloc(lcd->characters, sizeof(int8_t));
    gfx->buffer_ms = calloc(lcd->characters, sizeof(uint16_t));
    gfx->update = 0;
    // gfx->refresh_ms = 610;
    gfx->refresh_ms = 610 / 2;
    gfx->set_time = 0;

    // memset(gfx->sprites, sprite_blank, 8);
    gfx->sprites[0] = sprite_blank;
    gfx->sprites[1] = sprite_blank;
    gfx->sprites[2] = sprite_blank;
    gfx->sprites[3] = sprite_blank;
    gfx->sprites[4] = sprite_blank;
    gfx->sprites[5] = sprite_blank;
    gfx->sprites[6] = sprite_blank;
    gfx->sprites[7] = sprite_blank;
    memset(gfx->anim_sprite_counter, 0, sizeof(uint8_t) * 8);
    gfx->current_graphic.set = 0;

    gfx->play_anim_sprites = 1;
    gfx->animated_sprites_flags = 0;

    return gfx;
}

void lcdGFX_free(LcdGFX *gfx) {
    free(gfx);
}

static inline mask_update_t lcdGFX_hash(int i) {
    return 1 << (i % (sizeof(mask_update_t) * 8));
}

static inline gfx_int lcdGFX_xy2abs(LcdGFX *gfx, const gfx_int x, const gfx_int y) {
    return x + (y * gfx->lcd->columns);
}

void lcdGFX_initialize(LcdGFX *gfx) {
    pcf8574_lcd_initialize(gfx->lcd);
    lcdGFX_clear(gfx);
}

void lcdGFX_clear_buffer(LcdGFX *gfx) {
    memset(gfx->buffer, ' ', gfx->lcd->characters);
    memset(gfx->buffer_ms, 0, gfx->lcd->characters);
    gfx->update = 0xFFFFFFFF;
    gfx->refresh = 1;
}

void lcdGFX_clear(LcdGFX *gfx) {
    pcf8574_lcd_clear(gfx->lcd);
    lcdGFX_clear_buffer(gfx);
}

void lcdGFX_play_animated_sprites(LcdGFX *gfx);

// #define DISABLE_UPDATE_MASK
#define DISABLE_UPDATE_SEQ
// #define DEBUG_UPDATE_MASK

void lcdGFX_update(LcdGFX *gfx) {

    lcdGFX_play_animated_sprites(gfx);

    uint16_t c = 0;

    mask_update_t hold_update = 0;

#ifndef DISABLE_UPDATE_SEQ
    uint16_t nx = 0;
#endif
    for (size_t y = 0; y < gfx->lcd->characters / gfx->lcd->columns; y++) {
#ifndef DISABLE_UPDATE_MASK
        uint16_t ms = HAL_GetTick() % 0xFFFF;
#endif
        for (size_t x = 0; x < gfx->lcd->columns; x++) {
#ifdef DISABLE_UPDATE_MASK
            pcf8574_lcd_set_cursor(gfx->lcd, y, x);
            pcf8574_lcd_send_data(gfx->lcd, gfx->buffer[c]);
#else
    #ifdef DEBUG_UPDATE_MASK
            static uint8_t d_x = '0';
            if (gfx->buffer[c] && (lcdGFX_hash(c) & gfx->update)) {
                // if (nx != x || ny != y)
                pcf8574_lcd_set_cursor(gfx->lcd, y, x);
                pcf8574_lcd_send_data(gfx->lcd, d_x++);
                d_x %= ':';
                d_x = d_x ? d_x : '0';
                // nx = (x + 1) % gfx->lcd->columns;
                // ny = (y + (nx == 0)) % gfx->lcd->rows;
            }
    #else
            mask_update_t h = lcdGFX_hash(c);
            if (h & gfx->update || gfx->refresh) {
                if (gfx->buffer_ms[c] <= ms || gfx->refresh) {
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
                } else {
                    hold_update |= h;
                }
            }
    #endif
#endif
            c++;
        }
    }
    gfx->refresh = 0;
    gfx->update = hold_update;
}

void lcdGFX_set_char_abs(LcdGFX *gfx, gfx_int i, char c) {
    if ((i >= 0) && (i < gfx->lcd->characters) && (gfx->buffer[i] != c)) {
        gfx->buffer[i] = c;
        gfx->update |= lcdGFX_hash(i);
    }
}

void lcdGFX_print_char(LcdGFX *gfx, gfx_int x, gfx_int y, const char c) {
    gfx_int i = lcdGFX_xy2abs(gfx, x, y);
    lcdGFX_set_char_abs(gfx, i, c);
}

void lcdGFX_print_string(LcdGFX *gfx, gfx_int x, gfx_int y, const char *str, int8_t pad) {
    gfx_int i = lcdGFX_xy2abs(gfx, x, y);
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

void lcdGFX_draw_line(LcdGFX *gfx, gfx_int x0, gfx_int y0, gfx_int x1, gfx_int y1, const char c) {
    gfx_int max = gfx->lcd->columns;

    float dx = (abs(x0 - x1) * (x0 > x1 ? -1 : 1)) * 1.0f / max;
    float dy = (abs(y0 - y1) * (y0 > y1 ? -1 : 1)) * 1.0f / max;
    float x = x0, y = y0;

    for (gfx_int i = 0; i < max; i++) {
        lcdGFX_set_char_abs(gfx, lcdGFX_xy2abs(gfx, (x += dx), (y += dy)), c);
    }
}

void lcdGFX_draw_box(LcdGFX *gfx, gfx_int x, gfx_int y, gfx_int w, gfx_int h, const char c) {
    gfx_int y1 = y + h, x1 = x + w;
    for (; y < y1; y++) {
        for (x = x1 - w; x < x1; x++) {
            lcdGFX_set_char_abs(gfx, lcdGFX_xy2abs(gfx, x, y), c);
        }
    }
}

static void lcdGFX_erase_graphic(LcdGFX *gfx, gfx_int x, gfx_int y, gfx_int w, gfx_int h) {
    gfx_int c = 8;
    for (gfx_int _y = y; _y < y + h; _y++) {
        for (gfx_int _x = x; _x < x + w; _x++) {
            lcdGFX_set_char_abs(gfx, lcdGFX_xy2abs(gfx, _x, _y), ' ');
            if (!(--c))
                return;
        }
    }
}

void lcdGFX_play_animated_sprites(LcdGFX *gfx) {
    static uint32_t next = 0;
    uint32_t ticks_ms = HAL_GetTick();
    if (ticks_ms >= next) {
        if (!gfx->current_graphic.set && gfx->animated_sprites_flags) {
            for (uint8_t i = 0; i < 8; i++) {
                if (gfx->animated_sprites_flags & (1 << i)) {
                    pcf8574_lcd_create_char(gfx->lcd, i, ((uint8_t *)(gfx->sprites[i]) + gfx->anim_sprite_counter[i] * 8));
                    gfx->anim_sprite_counter[i]++;
                    gfx->anim_sprite_counter[i] %= 8;
                }
            }
        }
        next = ticks_ms + gfx->refresh_ms / 4;
    }
}

void lcdGFX_store_sprite(LcdGFX *gfx, const uint8_t sprite[8], uint8_t slot) {
    gfx->sprites[slot % 8] = sprite;
    pcf8574_lcd_create_char(gfx->lcd, slot % 8, sprite);
    gfx->animated_sprites_flags &= ~(1 << (slot % 8));
}

void lcdGFX_store_anim_sprite(LcdGFX *gfx, const uint8_t anim_sprite[8][8], uint8_t slot) {
    lcdGFX_store_sprite(gfx, (const uint8_t *)anim_sprite, slot); // Reinterpret later
    // gfx->animated_sprites[slot % 8] = anim_sprite;
    gfx->animated_sprites_flags |= 1 << (slot % 8);
}

void lcdGFX_clear_graphic(LcdGFX *gfx) {
    if (gfx->current_graphic.set) {
        lcdGFX_erase_graphic(gfx, gfx->current_graphic.x, gfx->current_graphic.y, gfx->current_graphic.w, gfx->current_graphic.h);
        lcdGFX_update(gfx);
        for (uint8_t i = 0; i < 8; i++) {
            pcf8574_lcd_create_char(gfx->lcd, i, gfx->sprites[i]);
        }
        gfx->current_graphic.set = 0;
    }
}

void lcdGFX_draw_graphic(LcdGFX *gfx, gfx_int x, gfx_int y, gfx_int w, gfx_int h, const uint8_t graphic[8][8]) {
    if (gfx->current_graphic.set) {
        lcdGFX_erase_graphic(gfx, gfx->current_graphic.x, gfx->current_graphic.y, gfx->current_graphic.w, gfx->current_graphic.h);
    }

    uint8_t c = 0;
    for (gfx_int _y = y; _y < y + h; _y++) {
        for (gfx_int _x = x; _x < x + w; _x++) {
            pcf8574_lcd_create_char(gfx->lcd, c, graphic[c]);
            lcdGFX_set_char_abs(gfx, lcdGFX_xy2abs(gfx, _x, _y), c);
            if ((++c) >= 8) {
                gfx->current_graphic.set = 1;
                gfx->current_graphic.x = x;
                gfx->current_graphic.y = y;
                gfx->current_graphic.w = w;
                gfx->current_graphic.h = h;
                return;
            }
        }
    }
}
