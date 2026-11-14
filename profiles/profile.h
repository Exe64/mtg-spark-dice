/* Game profiles (LittleFS JSON). Schema:
 *   name             string, 1..23 chars
 *   start_life       int 1..999
 *   family           string, 0..15 chars, optional (shown above the format: "Singleton", "Construit")
 *   family_en        string, 0..15 chars, optional: the family in English when it differs
 *   players          int 1..4 (default player count)
 *   players_min/max  int 2..4, optional (default 2..4); min == max = fixed table size.
 *                    A seat is whoever shares a life total: a two-headed giant team is one seat.
 *   commander_damage int 0..99, 0 = disabled (lethal threshold)
 *   poison           int 0..99, 0 = disabled (lethal threshold)
 */
#pragma once
#include <stdbool.h>

#include "game.h"

typedef struct {
    char name[24];
    char family[16], family_en[16];
    uint8_t players, players_min, players_max;
    sd_rules_t rules;
} sd_profile_t;

/* On failure returns false and sets *err (static string) if err is non-NULL. */
bool sd_profile_parse(const char *json, sd_profile_t *out, const char **err);
bool sd_profile_load(const char *path, sd_profile_t *out, const char **err);

/* Loads the shipped formats from `dir`, in the order the UI lists them; unreadable ones are reported on stderr and
 * skipped. Returns how many were loaded. */
#define SD_PROFILES_MAX 12
int sd_profiles_load(const char *dir, sd_profile_t *out, int max);
