/* Light primitives and icons (ui/fx.c): renders a specimen sheet and checks where the light falls.
 *   test_fx <out.png> */
#undef NDEBUG
#include <assert.h>
#include <stdio.h>

#include "fx.h"
#include "lvgl.h"
#include "shot.h"

#define BG 0x18214A
#define GLASS 0x0A1030
static const uint32_t SEAT[4] = {0xFF7A45, 0x3FD8C0, 0xFFC94D, 0xA98BFF};

static int luma(int x, int y) /* 0..765 */
{
    uint32_t c = shot_pixel(x, y);
    return (int)((c >> 16) & 0xFF) + (int)((c >> 8) & 0xFF) + (int)(c & 0xFF);
}
#define BG_LUMA (0x18 + 0x21 + 0x4A)
/* specimen layout, kept inside the round dial */
#define HALO_X(i) (75 + (i) * 70)
#define HALO_Y 92
#define RING_X 95
#define RAY_X 205
#define ICON_X(i) (120 + ((i) % 4) * 40)
#define ICON_Y(i) ((i) < 4 ? 30 : 322)

static void draw(lv_event_t *e)
{
    lv_layer_t *l = lv_event_get_layer(e);
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.bg_color = lv_color_hex(BG);
    lv_area_t a = {0, 0, SHOT_W - 1, SHOT_H - 1};
    lv_draw_rect(l, &d, &a);
    for (int i = 0; i < 4; i++) /* halos in the four seat hues */
        fx_halo(l, HALO_X(i), HALO_Y, 34, SEAT[i], LV_OPA_COVER);
    fx_ring(l, RING_X, 170, 48, 0xFFFFFF, LV_OPA_COVER);
    fx_ray(l, RAY_X, 170, 0, 100, 0xFFFFFF, LV_OPA_COVER);   /* right */
    fx_ray(l, RAY_X, 170, 90, 40, 0xFFFFFF, LV_OPA_COVER);   /* down */
    fx_ray(l, RAY_X, 170, -60, 100, SEAT[0], LV_OPA_COVER);  /* up-right */
    fx_glass(l, 150, 268, 26, GLASS, false);
    fx_icon(l, FX_ICON_SETTINGS, 150, 268, 0xF4F7FF, LV_OPA_COVER);
    fx_glass(l, 230, 268, 26, GLASS, true);
    fx_icon(l, FX_ICON_RESUME, 230, 268, 0xF4F7FF, LV_OPA_COVER);
    for (int i = 0; i < FX_ICON_COUNT; i++) /* the whole icon set */
        fx_icon(l, (fx_icon_t)i, ICON_X(i), ICON_Y(i), 0xF4F7FF, LV_OPA_COVER);
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    shot_init();
    lv_obj_t *o = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(o);
    lv_obj_set_size(o, SHOT_W, SHOT_H);
    lv_obj_add_event_cb(o, draw, LV_EVENT_DRAW_MAIN, NULL);
    shot_render();
    assert(shot_save(argv[1])); /* saved first: a failing check below still leaves the sheet to look at */

    /* halo: brightest at the centre, fading outward, nothing past its radius */
    for (int i = 0; i < 4; i++) {
        int cx = HALO_X(i), cy = HALO_Y;
        assert(luma(cx, cy) > luma(cx + 17, cy) && luma(cx + 17, cy) > luma(cx + 30, cy));
        assert(luma(cx, cy) > BG_LUMA + 250);
        assert(shot_pixel(cx, cy - 37) == BG && shot_pixel(cx - 26, cy - 26) == BG);
    }
    /* ring: light on the circle of radius 48, dark at its centre and outside */
    assert(luma(RING_X + 48, 170) > 600 && luma(RING_X, 170 - 48) > 600);
    assert(shot_pixel(RING_X, 170) == BG && shot_pixel(RING_X + 68, 170) == BG);
    /* rays leave (180,170) in the asked direction, fade with distance, and stay narrow */
    assert(luma(RAY_X + 25, 170) > luma(RAY_X + 75, 170) && luma(RAY_X + 75, 170) > BG_LUMA);
    assert(luma(RAY_X, 170 + 12) > BG_LUMA + 60);
    assert(shot_pixel(RAY_X + 104, 170) == BG && shot_pixel(RAY_X, 170 + 44) == BG); /* length respected */
    assert(shot_pixel(RAY_X - 30, 170 - 30) == BG);                                /* nothing behind */
    assert(luma(RAY_X + 20, 170 - 35) > BG_LUMA + 60);                             /* -60°: up-right */
    /* glass: darker than what surrounds it; only the focused button casts a halo */
    assert(luma(150 - 20, 268) < BG_LUMA && luma(150 - 32, 268) <= BG_LUMA + 3);
    assert(luma(230 - 32, 268) > BG_LUMA + 40 && luma(230 - 20, 268) < luma(230 - 32, 268));
    /* icons: each has ink, and keeps a clear 1 px margin inside its 40 px box */
    for (int i = 0; i < FX_ICON_COUNT; i++) {
        int cx = ICON_X(i), cy = ICON_Y(i), ink = 0;
        for (int y = cy - 20; y < cy + 20; y++)
            for (int x = cx - 20; x < cx + 20; x++) {
                bool lit = luma(x, y) > BG_LUMA + 200;
                ink += lit;
                assert(!(lit && (x == cx - 20 || x == cx + 19 || y == cy - 20 || y == cy + 19)));
            }
        assert(ink > 60);
    }
    printf("fx: halo, ring, rays, glass and %d icons ok -> %s\n", FX_ICON_COUNT, argv[1]);
    return 0;
}
