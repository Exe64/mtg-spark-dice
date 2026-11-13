#include "game.h"

#include <string.h>

#define SAVE_MAGIC 0x53444733u /* "SDG3": 6 players, partner commander, counters, states, format name */
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
    case SD_CMD:
    case SD_CMD2: {
        uint8_t *c = &p->cmd[a->src][a->kind == SD_CMD2];
        before = *c;
        after = clampi(before + a->amount, 0, SD_COUNTER_MAX);
        *c = (uint8_t)after;
        p->life = (int16_t)(p->life - (after - before)); /* commander damage is also life loss */
        return (int16_t)(after - before);
    }
    case SD_ASCEND:
        p->ascended = a->amount > 0;
        return a->amount;
    default:
        if (a->kind >= SD_POISON && a->kind < SD_ASCEND) {
            uint8_t *c = &p->counter[a->kind - SD_POISON];
            before = *c;
            after = clampi(before + a->amount, 0, SD_COUNTER_MAX);
            *c = (uint8_t)after;
            return (int16_t)(after - before);
        }
        return a->amount; /* SD_TURN and the table states do not live in a player */
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
    g->turn = 1;
    g->monarch = g->initiative = SD_NOBODY;
}

static void log_push(sd_game_t *g, sd_action_t a)
{
    a.turn = g->turn;
    if (g->log_len == SD_LOG_MAX) { /* ponytail: drop oldest by memmove, ring buffer if log grows large */
        memmove(g->log, g->log + 1, sizeof(g->log[0]) * (SD_LOG_MAX - 1));
        g->log_len--;
    }
    g->log[g->log_len++] = a;
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
    log_push(g, a);
    if (g->n_players > 1 && sd_player_lost(g, g->active) && !sd_game_over(g)) {
        sd_game_next_turn(g);
        g->log[g->log_len - 1].automatic = 1;
    }
    return true;
}

bool sd_game_adjust(sd_game_t *g, uint8_t player, sd_kind_t kind, uint8_t src, int16_t step, uint32_t now_ms)
{
    if (player >= g->n_players || src >= g->n_players || kind == SD_TURN || kind >= SD_ASCEND)
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

bool sd_game_next_turn(sd_game_t *g)
{
    sd_game_flush(g);
    int from = g->active, to = -1;
    for (int k = 1; k <= g->n_players; k++) {
        int j = (from + k) % g->n_players;
        if (!sd_player_lost(g, (uint8_t)j)) {
            to = j;
            break;
        }
    }
    if (to < 0)
        return false;
    sd_action_t a = {.player = (uint8_t)from, .kind = SD_TURN, .src = (uint8_t)to, .amount = to <= from ? 1 : 0};
    log_push(g, a); /* logged under the turn it ends */
    g->active = (uint8_t)to;
    g->turn = (uint16_t)(g->turn + a.amount);
    return true;
}

bool sd_game_pending_toggle_cmd(sd_game_t *g)
{
    sd_action_t *a = &g->pending;
    if (!g->has_pending || !g->rules.cmd_lethal || a->src == a->player)
        return false;
    if (a->kind == SD_LIFE)
        *a = (sd_action_t){.player = a->player, .kind = SD_CMD, .src = a->src, .amount = (int16_t)-a->amount};
    else if (a->kind == SD_CMD || a->kind == SD_CMD2)
        *a = (sd_action_t){.player = a->player, .kind = SD_LIFE, .src = a->src, .amount = (int16_t)-a->amount};
    else
        return false;
    return true;
}

/* the table state a kind names, NULL for the other kinds */
static uint8_t *state_of(sd_game_t *g, uint8_t kind)
{
    return kind == SD_MONARCH ? &g->monarch : kind == SD_INITIATIVE ? &g->initiative : kind == SD_DAYNIGHT ? &g->daynight : NULL;
}

static bool state_valid(const sd_game_t *g, uint8_t kind, uint8_t v)
{
    return kind == SD_DAYNIGHT ? v <= SD_DN_NIGHT : v < g->n_players || v == SD_NOBODY;
}

bool sd_game_set(sd_game_t *g, sd_kind_t kind, uint8_t value)
{
    sd_action_t a = {.player = value, .kind = (uint8_t)kind, .src = value};
    uint8_t *st = state_of(g, (uint8_t)kind);
    if (kind == SD_ASCEND ? value >= g->n_players : !st || !state_valid(g, (uint8_t)kind, value) || *st == value)
        return false;
    sd_game_flush(g);
    if (st) {
        a.src = *st;
        *st = value;
    } else {
        a.amount = g->p[value].ascended ? -1 : 1;
        apply(&g->p[value], &a);
    }
    log_push(g, a);
    return true;
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
    if (a.kind == SD_TURN) {
        g->active = a.player;
        g->turn = (uint16_t)(g->turn - a.amount);
        return a.automatic && g->log_len ? sd_game_undo(g) : true; /* take back its cause too */
    }
    uint8_t *st = state_of(g, a.kind);
    if (st) {
        *st = a.src;
        return true;
    }
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
    if (g->rules.poison_lethal && sd_counter(p, SD_POISON) >= g->rules.poison_lethal)
        return true;
    for (int s = 0; g->rules.cmd_lethal && s < g->n_players; s++)
        for (int c = 0; c < SD_CMD_SLOTS; c++) /* each commander counts apart, partners included */
            if (p->cmd[s][c] >= g->rules.cmd_lethal)
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
    if (tmp.n_players < 1 || tmp.n_players > SD_MAX_PLAYERS || tmp.log_len > SD_LOG_MAX || tmp.turn < 1 ||
        tmp.active >= tmp.n_players || tmp.format[sizeof(tmp.format) - 1] ||
        !state_valid(&tmp, SD_MONARCH, tmp.monarch) || !state_valid(&tmp, SD_INITIATIVE, tmp.initiative) ||
        !state_valid(&tmp, SD_DAYNIGHT, tmp.daynight))
        return false;
    for (int i = 0; i < tmp.log_len; i++) {
        const sd_action_t *a = &tmp.log[i];
        if (a->kind >= SD_KIND_COUNT)
            return false;
        if (state_of(&tmp, a->kind) ? !state_valid(&tmp, a->kind, a->player) || !state_valid(&tmp, a->kind, a->src)
                                    : a->player >= tmp.n_players || a->src >= tmp.n_players)
            return false;
    }
    *g = tmp;
    return true;
}
