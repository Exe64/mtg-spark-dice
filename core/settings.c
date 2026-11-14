#include "settings.h"

#include <string.h>

#define SAVE_MAGIC 0x31534453u /* "SDS1" */

static const struct {
    int16_t lo, hi, def;
} DEF[SD_S_COUNT] = {
    [SD_S_VIEW] = {0, 1, SD_VIEW_ARENA},
    [SD_S_TURN_ORDER] = {0, 1, 0},
    [SD_S_START_LIFE] = {0, SD_LIFE_MAX, 0},
    [SD_S_CLICKS] = {1, 12, 1},
    [SD_S_START_ON] = {0, 1, 0},
    [SD_S_ROTATE_SEATS] = {0, 1, 1},
    [SD_S_AUTO_KO] = {0, 1, 1},
    [SD_S_AUTO_WIN] = {0, 1, 1},
    [SD_S_CMD_LIFE] = {0, 1, 1},
    [SD_S_DRAG_CMD] = {0, 1, 0},
    [SD_S_CMD_LETHAL] = {0, SD_COUNTER_MAX, 0},
    [SD_S_POISON_LETHAL] = {0, SD_COUNTER_MAX, 0},
    [SD_S_CMD_SHOW] = {0, 1, 0},
    [SD_S_COMMIT_MS] = {300, 3000, SD_COMMIT_MS},
    [SD_S_PROFILE] = {0, 31, 0},
    [SD_S_LANG] = {0, 7, 0}, /* the UI wraps to the languages it has */
    [SD_S_PIN_BOOT] = {0, 1, 0},
    [SD_S_KNOB_REVERSE] = {0, 1, 0},
    [SD_S_KNOB_COUNTS] = {1, 8, 4},
    [SD_S_HAPTIC] = {0, 3, 3},
    [SD_S_SETUP_DONE] = {0, 1, 0},
};

/* Saved form: this header, then `n` int16 values in sd_setting_id_t order. */
typedef struct {
    uint32_t magic;
    uint16_t n;
    char name[24], owner_msg[32], pin[5];
} save_hdr_t;

int sd_setting_min(sd_setting_id_t id) { return DEF[id].lo; }
int sd_setting_max(sd_setting_id_t id) { return DEF[id].hi; }

void sd_setting_set(sd_settings_t *s, sd_setting_id_t id, int v)
{
    s->v[id] = (int16_t)(v < DEF[id].lo ? DEF[id].lo : v > DEF[id].hi ? DEF[id].hi : v);
}

void sd_settings_defaults(sd_settings_t *s)
{
    memset(s, 0, sizeof(*s));
    for (int i = 0; i < SD_S_COUNT; i++)
        s->v[i] = DEF[i].def;
}

bool sd_pin_valid(const char *pin)
{
    if (!pin[0])
        return true;
    for (int i = 0; i < 4; i++)
        if (pin[i] < '0' || pin[i] > '9')
            return false;
    return !pin[4];
}

size_t sd_settings_save_size(void) { return sizeof(save_hdr_t) + sizeof(((sd_settings_t *)0)->v); }

size_t sd_settings_save(const sd_settings_t *s, void *buf, size_t len)
{
    if (len < sd_settings_save_size())
        return 0;
    save_hdr_t h;
    memset(&h, 0, sizeof(h));
    h.magic = SAVE_MAGIC;
    h.n = SD_S_COUNT;
    memcpy(h.name, s->name, sizeof(h.name));
    memcpy(h.owner_msg, s->owner_msg, sizeof(h.owner_msg));
    memcpy(h.pin, s->pin, sizeof(h.pin));
    memcpy(buf, &h, sizeof(h));
    memcpy((uint8_t *)buf + sizeof(h), s->v, sizeof(s->v));
    return sd_settings_save_size();
}

bool sd_settings_load(sd_settings_t *s, const void *buf, size_t len)
{
    save_hdr_t h;
    sd_settings_defaults(s);
    if (len < sizeof(h))
        return false;
    memcpy(&h, buf, sizeof(h));
    if (h.magic != SAVE_MAGIC || len != sizeof(h) + (size_t)h.n * sizeof(int16_t))
        return false;
    for (int i = 0; i < h.n && i < SD_S_COUNT; i++) { /* a newer save's extra values are ignored */
        int16_t v;
        memcpy(&v, (const uint8_t *)buf + sizeof(h) + (size_t)i * sizeof(v), sizeof(v));
        sd_setting_set(s, (sd_setting_id_t)i, v);
    }
    h.name[sizeof(h.name) - 1] = h.owner_msg[sizeof(h.owner_msg) - 1] = h.pin[sizeof(h.pin) - 1] = '\0';
    memcpy(s->name, h.name, sizeof(s->name));
    memcpy(s->owner_msg, h.owner_msg, sizeof(s->owner_msg));
    if (sd_pin_valid(h.pin))
        memcpy(s->pin, h.pin, sizeof(s->pin));
    return true;
}

void sd_settings_rules(const sd_settings_t *s, sd_rules_t *r)
{
    if (s->v[SD_S_START_LIFE])
        r->start_life = s->v[SD_S_START_LIFE];
    if (r->cmd_lethal && s->v[SD_S_CMD_LETHAL])
        r->cmd_lethal = (uint8_t)s->v[SD_S_CMD_LETHAL];
    if (r->poison_lethal && s->v[SD_S_POISON_LETHAL])
        r->poison_lethal = (uint8_t)s->v[SD_S_POISON_LETHAL];
    r->opts = (uint8_t)((s->v[SD_S_AUTO_KO] ? 0 : SD_OPT_MANUAL_KO) | (s->v[SD_S_AUTO_WIN] ? 0 : SD_OPT_MANUAL_WIN) |
                        (s->v[SD_S_CMD_LIFE] ? 0 : SD_OPT_CMD_APART) | (s->v[SD_S_TURN_ORDER] ? SD_OPT_COUNTER_CW : 0));
    r->commit_ms = (uint16_t)s->v[SD_S_COMMIT_MS];
}

int sd_clicks_to_points(int16_t *rest, int detents, int clicks)
{
    int total = *rest + detents, points = total / clicks;
    *rest = (int16_t)(total - points * clicks);
    return points;
}
