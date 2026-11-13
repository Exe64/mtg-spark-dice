#include "fx.h"

#include <math.h>

extern const lv_image_dsc_t fx_ring_a8, fx_ray_a8, fx_icons_a8[FX_ICON_COUNT];
#define RING_NATIVE_R 48 /* RING_R in ui/assets/glow_gen.py */
#define GLASS_OPA 190
#define FOCUS_INK 0xFFFFFF

/* Tinted A8 mask whose pivot (px, py) lands on (x, y); scales are 256 = 1:1, rotation in 0.1°. */
static void mask(lv_layer_t *l, const lv_image_dsc_t *img, int x, int y, int px, int py, int sx, int sy, int rot,
                 uint32_t color, lv_opa_t opa)
{
    lv_draw_image_dsc_t d;
    lv_draw_image_dsc_init(&d);
    d.src = img;
    d.recolor = lv_color_hex(color);
    d.recolor_opa = LV_OPA_COVER;
    d.opa = opa;
    d.scale_x = sx;
    d.scale_y = sy;
    d.rotation = rot;
    d.pivot.x = px;
    d.pivot.y = py;
    lv_area_t a = {x - px, y - py, x - px + (int)img->header.w - 1, y - py + (int)img->header.h - 1};
    lv_draw_image(l, &d, &a);
}

static void centred(lv_layer_t *l, const lv_image_dsc_t *img, int cx, int cy, int scale, uint32_t color, lv_opa_t opa)
{
    mask(l, img, cx, cy, (int)img->header.w / 2, (int)img->header.h / 2, scale, scale, 0, color, opa);
}

/* Halo masks are computed once per radius, at their drawn size: scaling one mask every frame cost several times a
 * plain alpha blit (host/bench.c). Same falloff for every radius: a Gaussian that reaches zero at the radius.
 * ponytail: 6 radii cached, least recently used evicted; a screen drawing more than 6 distinct halo radii in one
 * frame would free a mask LVGL has not drawn yet. Today's maximum is 2; raise HALO_CACHE if a screen needs more. */
#define HALO_CACHE 6
static struct {
    int r;
    uint32_t used;
    lv_draw_buf_t *buf;
} halos[HALO_CACHE];
static uint32_t halo_clock;

static const lv_draw_buf_t *halo_mask(int r)
{
    int slot = 0;
    for (int i = 0; i < HALO_CACHE; i++) {
        if (halos[i].buf && halos[i].r == r) {
            halos[i].used = ++halo_clock;
            return halos[i].buf;
        }
        if (halos[i].used < halos[slot].used)
            slot = i;
    }
    if (halos[slot].buf)
        lv_draw_buf_destroy(halos[slot].buf);
    lv_draw_buf_t *b = halos[slot].buf = lv_draw_buf_create((uint32_t)(2 * r), (uint32_t)(2 * r), LV_COLOR_FORMAT_A8, 0);
    halos[slot].r = r;
    halos[slot].used = ++halo_clock;
    if (!b)
        return NULL;
    const float k = 4.5f, floor_ = expf(-k), c = r - 0.5f;
    for (int y = 0; y < 2 * r; y++)
        for (int x = 0; x < 2 * r; x++) {
            float d2 = ((x - c) * (x - c) + (y - c) * (y - c)) / ((float)r * r);
            float a = d2 < 1.f ? (expf(-k * d2) - floor_) / (1.f - floor_) : 0.f;
            b->data[(size_t)y * b->header.stride + x] = (uint8_t)(a * 255.f + 0.5f);
        }
    return b;
}

void fx_halo(lv_layer_t *l, int cx, int cy, int r, uint32_t color, lv_opa_t opa)
{
    const lv_draw_buf_t *m = r > 0 ? halo_mask(r) : NULL;
    if (m) /* no memory: the halo is decoration, never the only signal */
        centred(l, (const lv_image_dsc_t *)m, cx, cy, 256, color, opa);
}

void fx_ring(lv_layer_t *l, int cx, int cy, int r, uint32_t color, lv_opa_t opa)
{
    centred(l, &fx_ring_a8, cx, cy, 256 * r / RING_NATIVE_R, color, opa);
}

void fx_ray(lv_layer_t *l, int cx, int cy, float angle_deg, int len, uint32_t color, lv_opa_t opa)
{
    /* the mask points down (90°) from its top-centre pixel */
    mask(l, &fx_ray_a8, cx, cy, (int)fx_ray_a8.header.w / 2, 0, 256, 256 * len / (int)fx_ray_a8.header.h,
         (int)lroundf((angle_deg - 90.f) * 10.f), color, opa);
}

void fx_image(lv_layer_t *l, const lv_image_dsc_t *a8, int cx, int cy, uint32_t color, lv_opa_t opa)
{
    centred(l, a8, cx, cy, 256, color, opa);
}

void fx_icon(lv_layer_t *l, fx_icon_t icon, int cx, int cy, uint32_t color, lv_opa_t opa)
{
    centred(l, &fx_icons_a8[icon], cx, cy, 256, color, opa);
}

void fx_glass(lv_layer_t *l, int cx, int cy, int r, uint32_t fill, bool focused)
{
    if (focused)
        fx_halo(l, cx, cy, 2 * r, FOCUS_INK, LV_OPA_80);
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.bg_color = lv_color_hex(fill);
    d.bg_opa = GLASS_OPA;
    d.radius = LV_RADIUS_CIRCLE;
    d.border_color = lv_color_hex(FOCUS_INK);
    d.border_opa = focused ? LV_OPA_70 : LV_OPA_20;
    d.border_width = 1;
    lv_area_t a = {cx - r, cy - r, cx + r, cy + r};
    lv_draw_rect(l, &d, &a);
    lv_draw_arc_dsc_t h; /* top highlight: light catching the upper edge */
    lv_draw_arc_dsc_init(&h);
    h.color = lv_color_hex(FOCUS_INK);
    h.opa = LV_OPA_50;
    h.width = 1;
    h.center.x = cx;
    h.center.y = cy;
    h.radius = (uint16_t)r;
    h.start_angle = 205;
    h.end_angle = 335;
    lv_draw_arc(l, &h);
}
