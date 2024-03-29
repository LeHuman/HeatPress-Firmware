#pragma once

#include <stdint.h>

#include "main.h"
#include "pcf8574_lcd.h"

#define UPDATE_MASK_TYPE uint32_t

typedef struct LcdGFX {
    PCF8574_LCD *lcd;

    int refresh_ms;
    int set_time;

    int8_t *buffer;          // Buffer array
    uint16_t *buffer_ms;     // Buffer timeout array
    UPDATE_MASK_TYPE update; // Update Mask

} LcdGFX;

LcdGFX *lcdGFX_new(PCF8574_LCD *lcd);
void lcdGFX_free(LcdGFX *gfx);

void lcdGFX_initialize(LcdGFX *gfx);
void lcdGFX_update(LcdGFX *gfx);
void lcdGFX_clear(LcdGFX *gfx);

void lcdGFX_print_char(LcdGFX *gfx, int x, int y, char c);
void lcdGFX_print_string(LcdGFX *gfx, int x, int y, const char *str, int pad);

void lcdGFX_draw_line(LcdGFX *gfx, int8_t x0, int8_t y0, int8_t x1, int8_t y1, const char c);
void lcdGFX_draw_box(LcdGFX *gfx, int8_t x, int8_t y, int8_t w, int8_t h, const char c);
