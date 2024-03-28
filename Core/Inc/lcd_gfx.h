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
    UPDATE_MASK_TYPE update; // Update Mask

} LcdGFX;

LcdGFX *lcdGFX_new(PCF8574_LCD *lcd);
void lcdGFX_free(LcdGFX *gfx);

void lcdGFX_initialize(LcdGFX *gfx);
void lcdGFX_update(LcdGFX *gfx);
void lcdGFX_clear(LcdGFX *gfx);

void lcdGFX_set_char(LcdGFX *gfx, int x, int y, char c);
void lcdGFX_set_string(LcdGFX *gfx, int x, int y, char *str, int pad);