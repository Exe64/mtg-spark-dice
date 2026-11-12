/* Game profiles (LittleFS JSON). Schema:
 *   name             string, 1..23 chars
 *   start_life       int 1..999
 *   players          int 1..SD_MAX_PLAYERS (default player count)
 *   commander_damage int 0..99, 0 = disabled (lethal threshold)
 *   poison           int 0..99, 0 = disabled (lethal threshold)
 */
#pragma once
#include <stdbool.h>

#include "game.h"

typedef struct {
    char name[24];
    uint8_t players;
    sd_rules_t rules;
} sd_profile_t;

/* On failure returns false and sets *err (static string) if err is non-NULL. */
bool sd_profile_parse(const char *json, sd_profile_t *out, const char **err);
bool sd_profile_load(const char *path, sd_profile_t *out, const char **err);
