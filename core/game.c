#include "game.h"

#include <string.h>

#define SAVE_MAGIC 0x53444731u /* "SDG1" */
#define PERSIST_SIZE offsetof(sd_game_t, pending)

typedef struct {
    uint32_t magic;
    uint32_t size;
} save_hdr_t;

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

/* Apply an action to a player; returns the effective amount after clamping. */
static int16_t apply(sd_player_t *p, const sd_action_t *a)
{
    int before, after;
    switch (a->kind) {
    case SD_LIFE:
        before = p->life;
        after = clampi(before + a->amount, SD_LIFE_MIN, SD_LIFE_MAX);
        p->life = (int16_t)after;
        return (int16_t)(after - before);
    case SD_POISON:
        before = p->poison;
        after = clampi(before + a->amount, 0, SD_COUNTER_MAX);
        p->poison = (uint8_t)after;
        return (int16_t)(after - before);
    case SD_CMD:
        before = p->cmd[a->src];
        after = clampi(before + a->amount, 0, SD_COUNTER_MAX);
        p->cmd[a->src] = (uint8_t)after;
        p->life = (int16_t)(p->life - (after - before)); /* commander damage is also life loss */
        return (int16_t)(after - before);
    }
    return 0;
}

void sd_game_new(sd_game_t *g, const sd_rules_t *r, uint8_t n_players)
{
    memset(g, 0, sizeof(*g));
    g->rules = *r;
    g->n_players = (uint8_t)clampi(n_players, 1, SD_MAX_PLAYERS);
    for (int i = 0; i < g->n_players; i++)
        g->p[i].life = r->start_life;
}

bool sd_game_flush(sd_game_t *g)
{
    if (!g->has_pending)
        return false;
    g->has_pending = false;
    sd_action_t a = g->pending;
    a.amount = apply(&g->p[a.player], &a);
    if (a.amount == 0)
        return false;
    if (g->log_len == SD_LOG_MAX) { /* ponytail: drop oldest by memmove, ring buffer if log grows large */
        memmove(g->log, g->log + 1, sizeof(g->log[0]) * (SD_LOG_MAX - 1));
        g->log_len--;
    }
    g->log[g->log_len++] = a;
    return true;
}

bool sd_game_adjust(sd_game_t *g, uint8_t player, sd_kind_t kind, uint8_t src, int16_t step, uint32_t now_ms)
{
    if (player >= g->n_players || src >= g->n_players)
        return false;
    bool committed = false;
    sd_action_t *pa = &g->pending;
    if (g->has_pending && (pa->player != player || pa->kind != kind || pa->src != src))
        committed = sd_game_flush(g);
    if (!g->has_pending) {
        *pa = (sd_action_t){.player = player, .kind = (uint8_t)kind, .src = src, .amount = 0};
        g->has_pending = true;
    }
    pa->amount = (int16_t)clampi(pa->amount + step, -999, 999);
    g->pending_since = now_ms;
    return committed;
}

bool sd_game_tick(sd_game_t *g, uint32_t now_ms)
{
    if (!g->has_pending || (uint32_t)(now_ms - g->pending_since) < SD_COMMIT_MS)
        return false;
    return sd_game_flush(g);
}

bool sd_game_undo(sd_game_t *g)
{
    g->has_pending = false;
    if (g->log_len == 0)
        return false;
    sd_action_t a = g->log[--g->log_len];
    a.amount = (int16_t)-a.amount;
    apply(&g->p[a.player], &a);
    return true;
}

sd_player_t sd_game_view(const sd_game_t *g, uint8_t i)
{
    sd_player_t p = g->p[i];
    if (g->has_pending && g->pending.player == i)
        apply(&p, &g->pending);
    return p;
}

bool sd_player_lost(const sd_game_t *g, uint8_t i)
{
    const sd_player_t *p = &g->p[i];
    if (p->life <= 0)
        return true;
    if (g->rules.poison_lethal && p->poison >= g->rules.poison_lethal)
        return true;
    for (int s = 0; g->rules.cmd_lethal && s < g->n_players; s++)
        if (p->cmd[s] >= g->rules.cmd_lethal)
            return true;
    return false;
}

static int alive_count(const sd_game_t *g, int *last)
{
    int n = 0;
    for (int i = 0; i < g->n_players; i++)
        if (!sd_player_lost(g, (uint8_t)i)) {
            n++;
            *last = i;
        }
    return n;
}

bool sd_game_over(const sd_game_t *g)
{
    int last;
    int alive = alive_count(g, &last);
    return g->n_players == 1 ? alive == 0 : alive <= 1;
}

int sd_game_winner(const sd_game_t *g)
{
    int last = -1;
    return g->n_players > 1 && alive_count(g, &last) == 1 ? last : -1;
}

size_t sd_game_save_size(void) { return sizeof(save_hdr_t) + PERSIST_SIZE; }

size_t sd_game_save(const sd_game_t *g, void *buf, size_t len)
{
    if (len < sd_game_save_size())
        return 0;
    /* ponytail: raw struct image, same-firmware only; versioned by size+magic, explicit encoding if the
     * blob ever crosses devices/firmware layouts */
    save_hdr_t h = {SAVE_MAGIC, (uint32_t)PERSIST_SIZE};
    memcpy(buf, &h, sizeof(h));
    memcpy((uint8_t *)buf + sizeof(h), g, PERSIST_SIZE);
    return sd_game_save_size();
}

bool sd_game_load(sd_game_t *g, const void *buf, size_t len)
{
    save_hdr_t h;
    if (len != sd_game_save_size())
        return false;
    memcpy(&h, buf, sizeof(h));
    if (h.magic != SAVE_MAGIC || h.size != PERSIST_SIZE)
        return false;
    sd_game_t tmp;
    memset(&tmp, 0, sizeof(tmp));
    memcpy(&tmp, (const uint8_t *)buf + sizeof(h), PERSIST_SIZE);
    if (tmp.n_players < 1 || tmp.n_players > SD_MAX_PLAYERS || tmp.log_len > SD_LOG_MAX)
        return false;
    for (int i = 0; i < tmp.log_len; i++)
        if (tmp.log[i].player >= tmp.n_players || tmp.log[i].src >= tmp.n_players || tmp.log[i].kind > SD_CMD)
            return false;
    *g = tmp;
    return true;
}
