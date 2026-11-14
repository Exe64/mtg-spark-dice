/* SparkDice game core — pure C, no platform dependency (shared by firmware and host tests). */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SD_MAX_PLAYERS 6
#define SD_CMD_SLOTS 2 /* commanders per player: a partner pair deals damage that is counted apart */
#define SD_LOG_MAX 128
#define SD_COMMIT_MS 800 /* knob inactivity before a pending delta is committed, unless the rules say otherwise */
#define SD_LIFE_MIN (-999)
#define SD_LIFE_MAX 999
#define SD_COUNTER_MAX 99

typedef enum {
    SD_LIFE, /* src == player: own change; src != player: combat damage dealt by src */
    SD_CMD,  /* commander damage from src's first commander, also life loss */
    SD_CMD2, /* same, from src's second commander (partner) */
    SD_TURN, /* turn passed: player = previous active, src = new active, amount = turn increment (0/1) */
    /* counters, 0..SD_COUNTER_MAX each, kept in sd_player_t.counter[kind - SD_POISON]; only poison can be lethal */
    SD_POISON,
    SD_ENERGY,
    SD_EXPERIENCE,
    SD_TREASURE,
    SD_FOOD,
    SD_STORM,
    SD_RAD,
    /* states, changed at once by sd_game_set(): in the log, player = new value and src = previous value */
    SD_ASCEND,     /* the city's blessing of one player: player = src = that player, amount = +1 gained / -1 lost */
    SD_MONARCH,    /* holder: a player or SD_NOBODY */
    SD_INITIATIVE, /* holder: a player or SD_NOBODY */
    SD_DAYNIGHT,   /* sd_daynight_t */
    SD_KIND_COUNT
} sd_kind_t;
#define SD_N_COUNTERS (SD_ASCEND - SD_POISON)
#define SD_NOBODY 0xFF
typedef enum { SD_DN_NONE, SD_DN_DAY, SD_DN_NIGHT } sd_daynight_t;

typedef struct {
    int16_t start_life;
    uint8_t cmd_lethal;    /* 0 = commander damage disabled */
    uint8_t poison_lethal; /* 0 = poison disabled */
    uint8_t opts;          /* SD_OPT_*, 0 = the rules as printed */
    uint16_t commit_ms;    /* 0 = SD_COMMIT_MS */
} sd_rules_t;
#define SD_OPT_MANUAL_KO 1  /* nobody is defeated automatically: totals go to 0 and below, the table decides */
#define SD_OPT_MANUAL_WIN 2 /* the game never ends by itself */
#define SD_OPT_CMD_APART 4  /* commander damage is counted without touching the life total */
#define SD_OPT_COUNTER_CW 8 /* turns pass counter-clockwise */
#define SD_OPT_ALL 15

typedef struct {
    int16_t life;
    uint8_t counter[SD_N_COUNTERS];            /* see sd_counter() */
    uint8_t cmd[SD_MAX_PLAYERS][SD_CMD_SLOTS]; /* commander damage received, by source player and commander */
    uint8_t ascended;                          /* has the city's blessing */
} sd_player_t;

static inline uint8_t sd_counter(const sd_player_t *p, sd_kind_t counter) { return p->counter[counter - SD_POISON]; }

/* One committed (or pending) change. In the log, amount is the effective (post-clamp) value. */
typedef struct {
    uint8_t player;
    uint8_t kind; /* sd_kind_t */
    uint8_t src;  /* see sd_kind_t */
    uint8_t automatic; /* SD_TURN only: passed because the active player was just defeated (undone with its cause) */
    int16_t amount;
    uint16_t turn; /* turn during which the action happened */
} sd_action_t;

typedef struct {
    /* persisted part (see sd_game_save) */
    sd_rules_t rules;
    char format[24]; /* name of the format being played, set by the caller after sd_game_new(); may be empty */
    uint8_t n_players;
    sd_player_t p[SD_MAX_PLAYERS];
    uint16_t turn;  /* starts at 1 */
    uint8_t active; /* player whose turn it is */
    uint8_t monarch, initiative; /* holder, or SD_NOBODY */
    uint8_t daynight;            /* sd_daynight_t */
    uint16_t log_len;
    sd_action_t log[SD_LOG_MAX]; /* undo stack + action journal, oldest first */
    /* volatile part — must stay last */
    sd_action_t pending;
    bool has_pending;
    uint32_t pending_since;
} sd_game_t;

void sd_game_new(sd_game_t *g, const sd_rules_t *r, uint8_t n_players);

static inline uint16_t sd_commit_ms(const sd_game_t *g) { return g->rules.commit_ms ? g->rules.commit_ms : SD_COMMIT_MS; }

/* Accumulate `step` into the pending delta (life, commander damage and counters; not SD_TURN, not the states). A pending delta on another target is committed first.
 * Returns true if that implicit commit happened (caller should persist). */
bool sd_game_adjust(sd_game_t *g, uint8_t player, sd_kind_t kind, uint8_t src, int16_t step, uint32_t now_ms);

/* Commit the pending delta once sd_commit_ms() elapsed since the last adjust. True if committed. */
bool sd_game_tick(sd_game_t *g, uint32_t now_ms);

/* Commit the pending delta now. True if something was committed. If the commit defeats the active player (and the
 * game goes on), the turn passes automatically; undo reverts both at once. */
bool sd_game_flush(sd_game_t *g);

/* Pass the turn to the next player still in the game (defeated players are skipped), clockwise unless
 * SD_OPT_COUNTER_CW. The turn number increments when play comes back round to (or past) the first seat. Flushes any
 * pending delta first; logged, so undoable. */
bool sd_game_next_turn(sd_game_t *g);

/* Switch the pending delta between combat damage (SD_LIFE from another player) and commander damage (SD_CMD),
 * keeping its magnitude. False if nothing switchable is pending or commander damage is disabled. */
bool sd_game_pending_toggle_cmd(sd_game_t *g);

/* Change a state at once (no pending delay), committing any pending delta first. Logged, so undoable.
 *   SD_ASCEND: value = the player whose blessing is toggled
 *   SD_MONARCH, SD_INITIATIVE: value = the new holder, or SD_NOBODY
 *   SD_DAYNIGHT: value = sd_daynight_t
 * False if the value is invalid or changes nothing. Nothing follows automatically (no card draw, no hand-over when
 * the holder is defeated): the table decides. */
bool sd_game_set(sd_game_t *g, sd_kind_t kind, uint8_t value);

/* Revert the last committed delta (pending delta is discarded first). True if something was undone. */
bool sd_game_undo(sd_game_t *g);

/* Player state with the pending delta applied (what the screen shows). */
sd_player_t sd_game_view(const sd_game_t *g, uint8_t i);

bool sd_player_lost(const sd_game_t *g, uint8_t i);
bool sd_game_over(const sd_game_t *g);
int sd_game_winner(const sd_game_t *g); /* index of the last player standing, or -1 */

/* Snapshot for NVS. Pending delta is not saved: flush before saving. Returns bytes written, 0 if buf too small. */
size_t sd_game_save(const sd_game_t *g, void *buf, size_t len);
size_t sd_game_save_size(void);
/* False for anything that is not a save of this exact layout (older saves are not migrated: the game starts anew). */
bool sd_game_load(sd_game_t *g, const void *buf, size_t len);
