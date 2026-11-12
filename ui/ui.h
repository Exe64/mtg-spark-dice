/* SparkDice UI: screens, input handling and drawing on top of the game core.
 * Platform-neutral: the HAL (or the host screenshot harness) feeds input events and an ms clock. */
#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "game.h"
#include "profile.h"

#define UI_PRODUCT_NAME "SPARKDICE" /* working name: the only place it is written (PRODUCT.md, Brand Commitments) */
#define UI_BOOT_MS 1500             /* logo screen at power-on; any tap or knob step skips it */

#define UI_W 360
#define UI_H 360

typedef enum { UI_HAPTIC_DETENT, UI_HAPTIC_COMMIT, UI_HAPTIC_DEFEAT } ui_haptic_t;

typedef struct {
    void (*haptic)(ui_haptic_t pattern); /* may be NULL */
    void (*save)(const sd_game_t *g);    /* called after every committed delta, undo and new game; may be NULL */
} ui_hooks_t;

typedef enum { UI_SCR_HOME, UI_SCR_GAME, UI_SCR_MENU, UI_SCR_LOG, UI_SCR_SHEET, UI_SCR_OVER, UI_SCR_BOOT } ui_screen_t;

/* Call after lv_init() and display creation. `resume` (may be NULL) is offered as "REPRENDRE" on the home screen. */
void ui_init(const ui_hooks_t *hooks, const sd_profile_t *profiles, int n_profiles, const sd_game_t *resume);

/* Theme (ui_theme_id_t, see theme.h) and ground hue (index in ui_grounds[], used by themes that let the user pick).
 * Out-of-range values fall back to the defaults. May be called before ui_init() or at any time after. */
void ui_set_theme(int theme, int ground);

void ui_knob(int detents, uint32_t now_ms); /* +cw / -ccw */
void ui_tap(int x, int y, uint32_t now_ms);
void ui_long_press(int x, int y, uint32_t now_ms);
void ui_drag(int x0, int y0, int x1, int y1, uint32_t now_ms);
void ui_tick(uint32_t now_ms); /* call every frame: commits pending deltas, animates the crimp */

/* State inspection (tests, HAL) */
ui_screen_t ui_screen(void);
const sd_game_t *ui_game(void);
int ui_selected(void);
