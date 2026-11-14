/* User-facing strings, one table per language (strings_fr.c, strings_en.c). Adding a language = one such file and
 * one line in ui_langs[] (strings.c); host/test_strings.c checks every table is complete.
 * Entries ending in _FMT are printf formats: every language must keep the same conversions in the same order. */
#pragma once
#include <stdbool.h>

#define UI_STRINGS(X) \
    X(PLAY) X(RESUME) X(PLAYERS_FMT) X(LIFE_FMT) X(START) X(HOST) X(JOIN) X(RESUME_SUB_FMT) X(FORMAT) X(PLAYERS) \
    X(SOLO) X(NEXT) X(SEAT_FMT) X(SEAT_PROFILE_FMT) X(LIFE) X(POISON) X(CMD) X(CMD2) X(DEFEATED) X(MENU_UNDO) \
    X(MENU_LOG) X(MENU_SHEET_FMT) X(MENU_NEW) X(MENU_BACK) X(NOTHING_TO_UNDO) X(LOG_EMPTY) X(WINS_FMT) X(DEFEAT) \
    X(GAME_OVER) X(TURN_FMT) X(TAP_BACK) X(YES) X(NO) X(DAY) X(NIGHT) \
    X(GAME_SCREEN) X(ARENA) X(VIEW_ARENA_SUB) X(VIEW_SOLO_SUB) \
    /* markers: full name then chip label, each in sd_kind_t order from SD_POISON to SD_DAYNIGHT */ \
    X(MK_POISON) X(MK_ENERGY) X(MK_EXPERIENCE) X(MK_TREASURE) X(MK_FOOD) X(MK_STORM) X(MK_RAD) X(MK_ASCEND) \
    X(MK_MONARCH) X(MK_INITIATIVE) X(MK_DAYNIGHT) \
    X(MKA_POISON) X(MKA_ENERGY) X(MKA_EXPERIENCE) X(MKA_TREASURE) X(MKA_FOOD) X(MKA_STORM) X(MKA_RAD) X(MKA_ASCEND) \
    X(MKA_MONARCH) X(MKA_INITIATIVE) X(MKA_DAYNIGHT)

typedef enum {
#define X(id) STR_##id,
    UI_STRINGS(X)
#undef X
    STR_COUNT
} ui_str_t;

typedef struct {
    const char *name;       /* the language in its own words, as the picker shows it */
    const char *const *str; /* STR_COUNT entries */
    /* random device names: one noun and one adjective, at most 23 bytes together with the space */
    const char *const *noun, *const *adj;
    int n_noun, n_adj;
    bool adj_first;
} ui_lang_t;

#define UI_LANG_COUNT 2
extern const ui_lang_t *const ui_langs[UI_LANG_COUNT]; /* index = the SD_S_LANG setting: keep the order */
extern const ui_lang_t ui_lang_fr, ui_lang_en;
