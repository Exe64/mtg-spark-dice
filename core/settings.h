/* SparkDice settings — pure C, shared by firmware and host tests.
 * Every numeric setting lives in one int16 table indexed by sd_setting_id_t: defaults, bounds and the saved form all
 * come from the single table of settings.c. */
#pragma once
#include "game.h"

typedef enum {
    /* Append only, never reorder: the saved form is positional (sd_settings_save). */
    /* game */
    SD_S_VIEW,          /* sd_view_t: what a game with several players shows */
    SD_S_TURN_ORDER,    /* 0 clockwise, 1 counter-clockwise */
    SD_S_START_LIFE,    /* 0 = the format's, else 1..999 */
    SD_S_CLICKS,        /* knob detents per point, 1..12 */
    SD_S_START_ON,      /* 0 home, 1 straight into the saved game */
    SD_S_ROTATE_SEATS,  /* arena seats face their player; off = every numeral upright */
    SD_S_AUTO_KO,       /* a player at 0 life (or a lethal counter) is defeated */
    SD_S_AUTO_WIN,      /* the game ends when one player is left */
    SD_S_CMD_LIFE,      /* commander damage is also life loss */
    SD_S_DRAG_CMD,      /* a dragged attack starts as commander damage */
    SD_S_CMD_LETHAL,    /* 0 = the format's, else 1..99; never enables it in a format without commander damage */
    SD_S_POISON_LETHAL, /* same, for poison */
    SD_S_CMD_SHOW,      /* player sheet: 0 damage received (target), 1 damage dealt (source) */
    SD_S_COMMIT_MS,     /* knob inactivity before a pending delta is committed */
    SD_S_PROFILE,       /* format offered first on "new game" (index in the shipped list) */
    /* device */
    SD_S_LANG,          /* index in the UI's language table */
    SD_S_PIN_BOOT,      /* ask the PIN at power-on (only if one is set) */
    SD_S_KNOB_REVERSE,
    SD_S_KNOB_COUNTS,   /* quadrature counts per mechanical detent */
    SD_S_HAPTIC,        /* 0 off .. 3 strong */
    SD_S_SETUP_DONE,    /* the first-start assistant has been completed */
    SD_S_COUNT
} sd_setting_id_t;

typedef enum { SD_VIEW_ARENA, SD_VIEW_SOLO } sd_view_t;

typedef struct {
    int16_t v[SD_S_COUNT];
    char name[24];      /* device name */
    char owner_msg[32]; /* shown on the lock screen */
    char pin[5];        /* "" = none, else exactly 4 digits. Kept in clear: it deters, it does not protect. */
} sd_settings_t;

void sd_settings_defaults(sd_settings_t *s);
int sd_setting_min(sd_setting_id_t id);
int sd_setting_max(sd_setting_id_t id);
/* Set one value, clamped to its bounds. */
void sd_setting_set(sd_settings_t *s, sd_setting_id_t id, int v);
bool sd_pin_valid(const char *pin); /* "" or 4 digits */

/* Snapshot for NVS. Returns bytes written, 0 if buf is too small. */
size_t sd_settings_save(const sd_settings_t *s, void *buf, size_t len);
size_t sd_settings_save_size(void);
/* A save from an older firmware (fewer settings) loads, the new settings take their defaults; values are clamped.
 * False (and *s = defaults) for anything that is not a settings save. */
bool sd_settings_load(sd_settings_t *s, const void *buf, size_t len);

/* Turn a format's rules into the rules of a new game: the settings override what they set. */
void sd_settings_rules(const sd_settings_t *s, sd_rules_t *r);

/* Knob scaling: `clicks` detents make one point. `*rest` keeps the detents not yet spent between calls. */
int sd_clicks_to_points(int16_t *rest, int detents, int clicks);
