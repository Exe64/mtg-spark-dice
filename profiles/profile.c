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

bool sd_profile_parse(const char *json, sd_profile_t *out, const char **err)
{
    const char *e = NULL;
    int life, players, cmd, poison;
    cJSON *root = cJSON_Parse(json);
    const cJSON *name = cJSON_GetObjectItemCaseSensitive(root, "name");

    if (!cJSON_IsObject(root))
        e = "invalid JSON";
    else if (!cJSON_IsString(name) || !name->valuestring[0] || strlen(name->valuestring) >= sizeof(out->name))
        e = "name: string of 1..23 chars required";
    else if (!get_int(root, "start_life", 1, SD_LIFE_MAX, &life))
        e = "start_life: integer 1..999 required";
    else if (!get_int(root, "players", 1, SD_MAX_PLAYERS, &players))
        e = "players: integer 1..4 required";
    else if (!get_int(root, "commander_damage", 0, SD_COUNTER_MAX, &cmd))
        e = "commander_damage: integer 0..99 required";
    else if (!get_int(root, "poison", 0, SD_COUNTER_MAX, &poison))
        e = "poison: integer 0..99 required";

    if (!e) {
        memset(out, 0, sizeof(*out));
        strcpy(out->name, name->valuestring);
        out->players = (uint8_t)players;
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
