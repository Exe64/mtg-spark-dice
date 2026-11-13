/* Light and line icons of the luminous theme: A8 masks tinted at draw time. Ring, ray and icons are pre-rendered
 * (ui/assets/glow_gen.py); halos are computed once per radius and cached. No blur is computed per frame.
 * Colours are 0xRRGGBB. */
#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"

/* order = ICONS in ui/assets/glow_gen.py */
typedef enum {
    FX_ICON_SETTINGS,
    FX_ICON_PLAYERS,
    FX_ICON_RESUME,
    FX_ICON_GAME,
    FX_ICON_UNDO,
    FX_ICON_TURN,
    FX_ICON_WIFI, /* lot 2 */
    FX_ICON_ROOM, /* lot 3 */
    FX_ICON_COUNT
} fx_icon_t;
#define FX_ICON_PX 40

void fx_halo(lv_layer_t *l, int cx, int cy, int r, uint32_t color, lv_opa_t opa); /* soft disc of light, radius r */
void fx_ring(lv_layer_t *l, int cx, int cy, int r, uint32_t color, lv_opa_t opa); /* glowing ring of radius r */
/* Soft beam of length `len` leaving (cx, cy) toward `angle_deg` (0 = right, 90 = down). */
void fx_ray(lv_layer_t *l, int cx, int cy, float angle_deg, int len, uint32_t color, lv_opa_t opa);
/* Any A8 mask (logo, marks), centred on (cx, cy) at its native size. */
void fx_image(lv_layer_t *l, const lv_image_dsc_t *a8, int cx, int cy, uint32_t color, lv_opa_t opa);
void fx_icon(lv_layer_t *l, fx_icon_t icon, int cx, int cy, uint32_t color, lv_opa_t opa);
/* Round glass button: translucent dark fill, 1 px top highlight; a white halo when focused (never a seat hue). */
void fx_glass(lv_layer_t *l, int cx, int cy, int r, uint32_t fill, bool focused);
