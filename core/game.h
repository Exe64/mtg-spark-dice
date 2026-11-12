/* SparkDice game core — pure C, no platform dependency (shared by firmware and host tests). */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SD_MAX_PLAYERS 4
#define SD_LOG_MAX 128
#define SD_COMMIT_MS 800 /* knob inactivity before a pending delta is committed */
#define SD_LIFE_MIN (-999)
#define SD_LIFE_MAX 999
#define SD_COUNTER_MAX 99

typedef enum {
    SD_LIFE,   /* src == player: own change; src != player: combat damage dealt by src */
    SD_POISON,
    SD_CMD,    /* commander damage from src, also life loss */
    SD_TURN,   /* turn passed: player = previous active, src = new active, amount = turn increment (0/1) */
} sd_kind_t;

typedef struct {
    int16_t start_life;
    uint8_t cmd_lethal;    /* 0 = commander damage disabled */
    uint8_t poison_lethal; /* 0 = poison disabled */
} sd_rules_t;

typedef struct {
    int16_t life;
    uint8_t poison;
    uint8_t cmd[SD_MAX_PLAYERS]; /* commander damage received, indexed by source player */
} sd_player_t;

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
    uint8_t n_players;
    sd_player_t p[SD_MAX_PLAYERS];
    uint16_t turn;  /* starts at 1 */
    uint8_t active; /* player whose turn it is */
    uint16_t log_len;
    sd_action_t log[SD_LOG_MAX]; /* undo stack + action journal, oldest first */
    /* volatile part — must stay last */
    sd_action_t pending;
    bool has_pending;
    uint32_t pending_since;
} sd_game_t;

void sd_game_new(sd_game_t *g, const sd_rules_t *r, uint8_t n_players);

/* Accumulate `step` into the pending delta. A pending delta on another target is committed first.
 * Returns true if that implicit commit happened (caller should persist). */
bool sd_game_adjust(sd_game_t *g, uint8_t player, sd_kind_t kind, uint8_t src, int16_t step, uint32_t now_ms);

/* Commit the pending delta once SD_COMMIT_MS elapsed since the last adjust. True if committed. */
bool sd_game_tick(sd_game_t *g, uint32_t now_ms);

/* Commit the pending delta now. True if something was committed. If the commit defeats the active player (and the
 * game goes on), the turn passes automatically; undo reverts both at once. */
bool sd_game_flush(sd_game_t *g);

/* Pass the turn clockwise to the next player still in the game (defeated players are skipped). The turn number
 * increments when play wraps around to a lower seat. Flushes any pending delta first; logged, so undoable. */
bool sd_game_next_turn(sd_game_t *g);

/* Switch the pending delta between combat damage (SD_LIFE from another player) and commander damage (SD_CMD),
 * keeping its magnitude. False if nothing switchable is pending or commander damage is disabled. */
bool sd_game_pending_toggle_cmd(sd_game_t *g);

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
bool sd_game_load(sd_game_t *g, const void *buf, size_t len);
