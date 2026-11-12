#include "shot.h"

#include <stdio.h>
#include <stdlib.h>

#include "lvgl.h"
#include "libs/lodepng/lodepng.h"

#define BEZEL 0x0B0B0Cu

static uint32_t fb[SHOT_W * SHOT_H]; /* XRGB8888, rendered in place (FULL mode) */
static uint32_t now_ms;

static uint32_t tick_cb(void) { return now_ms; }
static void flush_cb(lv_display_t *d, const lv_area_t *a, uint8_t *px)
{
    (void)a;
    (void)px;
    lv_display_flush_ready(d);
}

void shot_init(void)
{
    lv_init();
    lv_tick_set_cb(tick_cb);
    lv_display_t *d = lv_display_create(SHOT_W, SHOT_H);
    lv_display_set_color_format(d, LV_COLOR_FORMAT_XRGB8888);
    lv_display_set_buffers(d, fb, NULL, sizeof(fb), LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(d, flush_cb);
}

void shot_set_time(uint32_t ms)
{
    now_ms = ms;
    lv_timer_handler();
}

void shot_render(void)
{
    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(NULL);
}

uint32_t shot_pixel(int x, int y) { return fb[y * SHOT_W + x] & 0xFFFFFFu; }

bool shot_save(const char *png_path)
{
    unsigned char *rgba = malloc(SHOT_W * SHOT_H * 4);
    if (!rgba)
        return false;
    const int r = SHOT_W / 2;
    for (int y = 0; y < SHOT_H; y++)
        for (int x = 0; x < SHOT_W; x++) {
            int dx = 2 * x + 1 - SHOT_W, dy = 2 * y + 1 - SHOT_H; /* doubled, pixel-center coords */
            uint32_t c = dx * dx + dy * dy > 4 * r * r ? BEZEL : shot_pixel(x, y);
            unsigned char *o = rgba + 4 * (y * SHOT_W + x);
            o[0] = (unsigned char)(c >> 16);
            o[1] = (unsigned char)(c >> 8);
            o[2] = (unsigned char)c;
            o[3] = 255;
        }
    /* LVGL's lodepng port writes files through lv_fs; encode in memory and use stdio instead */
    unsigned char *png = NULL;
    size_t png_len = 0;
    unsigned err = lodepng_encode32(&png, &png_len, rgba, SHOT_W, SHOT_H);
    free(rgba);
    FILE *f = err ? NULL : fopen(png_path, "wb");
    bool ok = f && fwrite(png, 1, png_len, f) == png_len;
    if (f)
        ok = (fclose(f) == 0) && ok;
    lv_free(png);
    if (!ok)
        fprintf(stderr, "png %s: %s\n", png_path, err ? lodepng_error_text(err) : "write failed");
    return ok;
}
