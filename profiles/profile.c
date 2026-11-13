#include "profile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

#define PROFILE_MAX_BYTES 2048

static bool get_int(const cJSON *root, const char *key, int lo, int hi, int *out)
{
    const cJSON *v = cJSON_GetObjectItemCaseSensitive(root, key);
    if (!cJSON_IsNumber(v) || v->valuedouble != (double)v->valueint || v->valueint < lo || v->valueint > hi)
        return false;
    *out = v->valueint;
    return true;
}

/* optional integer: absent = default, present = must be valid */
static bool opt_int(const cJSON *root, const char *key, int lo, int hi, int def, int *out)
{
    *out = def;
    return !cJSON_GetObjectItemCaseSensitive(root, key) || get_int(root, key, lo, hi, out);
}

bool sd_profile_parse(const char *json, sd_profile_t *out, const char **err)
{
    const char *e = NULL;
    int life, players, cmd, poison, pmin = 0, pmax = 0;
    cJSON *root = cJSON_Parse(json);
    const cJSON *name = cJSON_GetObjectItemCaseSensitive(root, "name");
    const cJSON *family = cJSON_GetObjectItemCaseSensitive(root, "family");

    if (!cJSON_IsObject(root))
        e = "invalid JSON";
    else if (!cJSON_IsString(name) || !name->valuestring[0] || strlen(name->valuestring) >= sizeof(out->name))
        e = "name: string of 1..23 chars required";
    else if (!get_int(root, "start_life", 1, SD_LIFE_MAX, &life))
        e = "start_life: integer 1..999 required";
    else if (!get_int(root, "players", 1, SD_MAX_PLAYERS, &players))
        e = "players: integer 1..4 required";
    else if (family && (!cJSON_IsString(family) || strlen(family->valuestring) >= sizeof(out->family)))
        e = "family: string of 0..15 chars";
    else if (!opt_int(root, "players_min", 2, SD_MAX_PLAYERS, 2, &pmin) ||
             !opt_int(root, "players_max", 2, SD_MAX_PLAYERS, SD_MAX_PLAYERS, &pmax) || pmin > pmax)
        e = "players_min/players_max: integers 2..4, min <= max";
    else if (!get_int(root, "commander_damage", 0, SD_COUNTER_MAX, &cmd))
        e = "commander_damage: integer 0..99 required";
    else if (!get_int(root, "poison", 0, SD_COUNTER_MAX, &poison))
        e = "poison: integer 0..99 required";

    if (!e) {
        memset(out, 0, sizeof(*out));
        strcpy(out->name, name->valuestring);
        if (family)
            strcpy(out->family, family->valuestring);
        out->players = (uint8_t)players;
        out->players_min = (uint8_t)pmin;
        out->players_max = (uint8_t)pmax;
        out->rules = (sd_rules_t){.start_life = (int16_t)life, .cmd_lethal = (uint8_t)cmd, .poison_lethal = (uint8_t)poison};
    }
    cJSON_Delete(root);
    if (e && err)
        *err = e;
    return !e;
}

bool sd_profile_load(const char *path, sd_profile_t *out, const char **err)
{
    char buf[PROFILE_MAX_BYTES + 1];
    FILE *f = fopen(path, "rb");
    if (!f) {
        if (err)
            *err = "cannot open file";
        return false;
    }
    size_t n = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    if (n > PROFILE_MAX_BYTES) {
        if (err)
            *err = "file too large";
        return false;
    }
    buf[n] = '\0';
    return sd_profile_parse(buf, out, err);
}

int sd_profiles_load(const char *dir, sd_profile_t *out, int max)
{
    static const char *const files[] = {"commander", "commander-1v1", "commander-2hg", "duel-commander", "standard",
                                        "pioneer",   "modern",        "legacy",        "pauper",         "peasant"};
    int n = 0;
    for (size_t i = 0; i < sizeof(files) / sizeof(files[0]) && n < max; i++) {
        char path[256];
        const char *err = "";
        snprintf(path, sizeof(path), "%s/%s.json", dir, files[i]);
        if (sd_profile_load(path, &out[n], &err))
            n++;
        else
            fprintf(stderr, "profile %s: %s\n", path, err);
    }
    return n;
}
