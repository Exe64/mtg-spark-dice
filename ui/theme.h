/* Visual tokens. Two themes share one layout and one set of gestures: Booster (plan #1, kept as shipped) and
 * Lumineux (plan #5). The selector lives in the settings (plan #3).
 * ponytail: tokens plus one `light` switch between the two drawing styles; make it per-screen draw functions in the
 * theme if a third style ever lands */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    bool light;           /* seats are light on glass (halos, rays, glass hub) instead of painted fields */
    uint32_t ground;      /* ground colour at the centre of the dial */
    uint32_t ground_rim;  /* ground colour at the rim (radial gradient); equal to `ground` = flat */
    bool ground_pick;     /* the user picks the ground among ui_grounds[]; the rim is derived from the hue */
    uint32_t ink;         /* text on fields and ground */
    uint32_t seat[4];     /* seat fields J1..J4 */
    uint32_t seat_ink[4]; /* text on each seat field (contrast >= 4.5:1) */
    uint32_t foil_a, foil_b, foil_ink; /* pending band only */
    int rule_w;           /* every rule, crimp and divider */
    const uint8_t *display_ttf, *display_ttf_end; /* numerals */
    const uint8_t *label_ttf, *label_ttf_end;     /* uppercase labels */
} ui_theme_t;

/* Stored as numbers outside the game save: keep the order. */
typedef enum { UI_THEME_BOOSTER, UI_THEME_LUMINEUX, UI_THEME_COUNT } ui_theme_id_t;
#define UI_THEME_DEFAULT UI_THEME_LUMINEUX

extern const ui_theme_t ui_theme_booster, ui_theme_lumineux;

/* Ground hues the user may pick (Nuit, Noir, Améthyste, Forêt, Grenat, Ardoise); index 0 is the default.
 * All are dark enough to keep the luminous ink at 7:1 or more (checked in host/shots.c). */
#define UI_GROUND_COUNT 6
extern const uint32_t ui_grounds[UI_GROUND_COUNT];
uint32_t ui_ground_rim(uint32_t hue); /* rim tone of the radial ground, derived from the centre hue */
