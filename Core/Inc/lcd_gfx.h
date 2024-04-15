#pragma once

#include <stdint.h>

#include "main.h"
#include "pcf8574_lcd.h"
#include "stdint-gcc.h" // BROKEN: Remove me

#define UPDATE_MASK_TYPE uint32_t

typedef int16_t gfx_int;

typedef struct LcdGFX {
    PCF8574_LCD *lcd;

    int refresh_ms;
    int set_time;

    const uint8_t *sprites[8];
    // uint8_t *animated_sprites[8];
    uint8_t anim_sprite_counter[8];
    uint8_t animated_sprites_flags;
    uint8_t play_anim_sprites;

    struct current_graphic {
        uint8_t set;
        gfx_int x;
        gfx_int y;
        gfx_int w;
        gfx_int h;
    } current_graphic;

    int8_t *buffer;          // Buffer array
    uint16_t *buffer_ms;     // Buffer timeout array
    UPDATE_MASK_TYPE update; // Update Mask

} LcdGFX;

LcdGFX *lcdGFX_new(PCF8574_LCD *lcd);
void lcdGFX_free(LcdGFX *gfx);

void lcdGFX_initialize(LcdGFX *gfx);
void lcdGFX_update(LcdGFX *gfx);
void lcdGFX_clear(LcdGFX *gfx);
void lcdGFX_clear_buffer(LcdGFX *gfx);

void lcdGFX_print_char(LcdGFX *gfx, gfx_int x, gfx_int y, char c);
void lcdGFX_print_string(LcdGFX *gfx, gfx_int x, gfx_int y, const char *str, int8_t pad);

void lcdGFX_store_sprite(LcdGFX *gfx, const uint8_t sprite[8], uint8_t slot);              // slot can only be 0-7
void lcdGFX_store_anim_sprite(LcdGFX *gfx, const uint8_t anim_sprite[8][8], uint8_t slot); // slot can only be 0-7

void lcdGFX_draw_line(LcdGFX *gfx, gfx_int x0, gfx_int y0, gfx_int x1, gfx_int y1, const char c);
void lcdGFX_draw_box(LcdGFX *gfx, gfx_int x, gfx_int y, gfx_int w, gfx_int h, const char c);
void lcdGFX_draw_graphic(LcdGFX *gfx, gfx_int x, gfx_int y, gfx_int w, gfx_int h, const uint8_t graphic[8][8]); // w*h can be at most 8
void lcdGFX_clear_graphic(LcdGFX *gfx);
