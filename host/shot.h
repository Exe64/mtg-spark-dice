/* Headless LVGL display + PNG snapshots (host only). */
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define SHOT_W 360
#define SHOT_H 360

void shot_init(void);
void shot_set_time(uint32_t ms);       /* fake LVGL tick, for deterministic animations */
void shot_render(void);                /* full redraw of the active screen */
uint32_t shot_pixel(int x, int y);     /* 0xRRGGBB of the last render */
bool shot_save(const char *png_path);  /* round mask applied: outside the dial painted as bezel */
