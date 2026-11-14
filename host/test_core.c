#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "settings.h"

static const sd_rules_t CMDR = {.start_life = 40, .cmd_lethal = 21, .poison_lethal = 10};

static void test_delta_commit(void)
{
    sd_game_t g;
    sd_game_new(&g, &CMDR, 4);
    assert(g.p[3].life == 40 && g.p[0].life == 40);

    /* 3 knob steps, no commit before 800 ms of inactivity */
    sd_game_adjust(&g, 0, SD_LIFE, 0, -1, 1000);
    sd_game_adjust(&g, 0, SD_LIFE, 0, -1, 1300);
    sd_game_adjust(&g, 0, SD_LIFE, 0, -1, 1600);
    assert(g.p[0].life == 40);               /* not committed */
    assert(sd_game_view(&g, 0).life == 37);  /* but shown */
    assert(!sd_game_tick(&g, 2399));         /* 799 ms after last step */
    assert(sd_game_tick(&g, 2400));          /* 800 ms */
    assert(g.p[0].life == 37 && g.log_len == 1 && g.log[0].amount == -3);
    assert(!sd_game_tick(&g, 5000));         /* nothing pending */

    /* back to zero = no log entry */
    sd_game_adjust(&g, 0, SD_LIFE, 0, +2, 6000);
    sd_game_adjust(&g, 0, SD_LIFE, 0, -2, 6100);
    assert(!sd_game_tick(&g, 7000) && g.log_len == 1);

    /* changing target commits the previous pending delta */
    sd_game_adjust(&g, 1, SD_LIFE, 0, -5, 8000);
    assert(sd_game_adjust(&g, 2, SD_LIFE, 0, -1, 8100));
    assert(g.p[1].life == 35 && g.log_len == 2);

    /* clock wrap-around */
    sd_game_flush(&g);
    sd_game_adjust(&g, 0, SD_LIFE, 0, 1, 0xFFFFFF00u);
    assert(!sd_game_tick(&g, 0x100));
    assert(sd_game_tick(&g, 0x300));

    /* invalid player ignored */
    assert(!sd_game_adjust(&g, 4, SD_LIFE, 0, 1, 0) && !g.has_pending);
}

static void test_undo(void)
{
    sd_game_t g;
    sd_game_new(&g, &CMDR, 2);
    assert(!sd_game_undo(&g));
    sd_game_adjust(&g, 0, SD_LIFE, 0, -10, 0);
    sd_game_flush(&g);
    sd_game_adjust(&g, 1, SD_POISON, 0, 3, 0);
    sd_game_flush(&g);
    sd_game_adjust(&g, 1, SD_LIFE, 0, -1, 0); /* pending: discarded by undo */
    assert(sd_game_undo(&g));
    assert(sd_counter(&g.p[1], SD_POISON) == 0 && g.p[1].life == 40 && !g.has_pending);
    assert(sd_game_undo(&g) && g.p[0].life == 40 && g.log_len == 0);

    /* clamped action undoes exactly */
    sd_game_adjust(&g, 0, SD_POISON, 0, -5, 0);
    assert(!sd_game_flush(&g) && g.log_len == 0); /* poison can't go below 0: no-op */
    sd_game_adjust(&g, 0, SD_LIFE, 0, 999, 0);
    sd_game_flush(&g);
    assert(g.p[0].life == SD_LIFE_MAX && g.log[0].amount == SD_LIFE_MAX - 40);
    sd_game_undo(&g);
    assert(g.p[0].life == 40);

    /* log overflow drops the oldest entry */
    for (int i = 0; i < SD_LOG_MAX + 5; i++) {
        sd_game_adjust(&g, 0, SD_LIFE, 0, (i & 1) ? 1 : -1, 0);
        sd_game_flush(&g);
    }
    assert(g.log_len == SD_LOG_MAX);
}

static void test_defeats(void)
{
    sd_game_t g;
    sd_game_new(&g, &CMDR, 3);
    assert(!sd_game_over(&g) && sd_game_winner(&g) == -1);

    sd_game_adjust(&g, 0, SD_LIFE, 0, -40, 0);
    sd_game_flush(&g);
    assert(sd_player_lost(&g, 0) && !sd_game_over(&g));

    sd_game_adjust(&g, 1, SD_POISON, 0, 9, 0);
    sd_game_flush(&g);
    assert(!sd_player_lost(&g, 1));
    sd_game_adjust(&g, 1, SD_POISON, 0, 1, 0);
    sd_game_flush(&g);
    assert(sd_player_lost(&g, 1));
    assert(sd_game_over(&g) && sd_game_winner(&g) == 2);

    sd_game_undo(&g); /* undo revives */
    assert(!sd_player_lost(&g, 1) && !sd_game_over(&g));

    /* commander damage: 21 from one source is lethal and also costs life */
    sd_game_new(&g, &CMDR, 4);
    sd_game_adjust(&g, 2, SD_CMD, 1, 20, 0);
    sd_game_flush(&g);
    assert(g.p[2].cmd[1][0] == 20 && g.p[2].life == 20 && !sd_player_lost(&g, 2));
    sd_game_adjust(&g, 2, SD_CMD, 3, 20, 0); /* other source: separate counter */
    sd_game_flush(&g);
    assert(g.p[2].life == 0 && sd_player_lost(&g, 2)); /* dead by life, not commander */
    sd_game_undo(&g);
    sd_game_adjust(&g, 2, SD_LIFE, 0, 30, 0);
    sd_game_flush(&g);
    sd_game_adjust(&g, 2, SD_CMD, 1, 1, 0);
    sd_game_flush(&g);
    assert(g.p[2].cmd[1][0] == 21 && g.p[2].life == 49 && sd_player_lost(&g, 2));
    sd_game_undo(&g);
    assert(g.p[2].cmd[1][0] == 20 && g.p[2].life == 50);
    assert(sd_game_view(&g, 2).cmd[1][0] == 20);

    /* pending commander damage shows on life */
    sd_game_adjust(&g, 2, SD_CMD, 1, 1, 0);
    assert(sd_game_view(&g, 2).life == 49 && sd_game_view(&g, 2).cmd[1][0] == 21);

    /* standard rules: commander damage disabled */
    sd_rules_t std = {.start_life = 20, .cmd_lethal = 0, .poison_lethal = 10};
    sd_game_new(&g, &std, 2);
    sd_game_adjust(&g, 0, SD_CMD, 1, 25, 0);
    sd_game_flush(&g);
    assert(g.p[0].life == -5 && sd_player_lost(&g, 0));

    /* solo: over when the single player loses */
    sd_game_new(&g, &std, 1);
    assert(!sd_game_over(&g));
    sd_game_adjust(&g, 0, SD_LIFE, 0, -20, 0);
    sd_game_flush(&g);
    assert(sd_game_over(&g) && sd_game_winner(&g) == -1);
}

static void test_save_load(void)
{
    sd_game_t g, h;
    unsigned char buf[8192];
    assert(sd_game_save_size() <= sizeof(buf));
    sd_game_new(&g, &CMDR, 4);
    sd_game_adjust(&g, 3, SD_CMD, 0, 7, 0);
    sd_game_flush(&g);
    sd_game_adjust(&g, 1, SD_POISON, 0, 2, 0);
    sd_game_flush(&g);
    size_t n = sd_game_save(&g, buf, sizeof(buf));
    assert(n == sd_game_save_size());
    assert(sd_game_load(&h, buf, n));
    assert(h.n_players == 4 && h.p[3].cmd[0][0] == 7 && h.p[3].life == 33 && sd_counter(&h.p[1], SD_POISON) == 2);
    assert(h.log_len == 2 && h.rules.start_life == 40 && !h.has_pending);
    assert(sd_game_undo(&h) && sd_counter(&h.p[1], SD_POISON) == 0);

    assert(sd_game_save(&g, buf, 10) == 0);
    assert(!sd_game_load(&h, buf, n - 1));
    buf[0] ^= 0xFF; /* bad magic */
    assert(!sd_game_load(&h, buf, n));
    buf[0] ^= 0xFF;
    sd_game_t bad = g;
    bad.log[0].player = 9;
    sd_game_save(&bad, buf, sizeof(buf));
    assert(!sd_game_load(&h, buf, n));
}

static void test_turns(void)
{
    sd_game_t g, h;
    unsigned char buf[8192];
    sd_game_new(&g, &CMDR, 4);
    assert(g.turn == 1 && g.active == 0);
    assert(sd_game_next_turn(&g) && g.active == 1 && g.turn == 1);
    sd_game_next_turn(&g);
    sd_game_next_turn(&g);
    assert(g.active == 3 && g.turn == 1);
    assert(sd_game_next_turn(&g) && g.active == 0 && g.turn == 2); /* wrap = new turn */
    assert(g.log[g.log_len - 1].kind == SD_TURN && g.log[g.log_len - 1].turn == 1);

    /* actions carry their turn; pending is flushed by next_turn */
    sd_game_adjust(&g, 2, SD_LIFE, 0, -5, 0); /* J1 attacks J3 */
    assert(sd_game_next_turn(&g) && g.p[2].life == 35 && g.log[g.log_len - 2].turn == 2);

    /* defeated players are skipped, and a wrap still counts */
    sd_game_adjust(&g, 2, SD_LIFE, 2, -35, 0);
    sd_game_flush(&g);
    assert(sd_player_lost(&g, 2));
    assert(g.active == 1 && sd_game_next_turn(&g) && g.active == 3);

    /* undo walks the turn back */
    uint16_t turn = g.turn;
    assert(sd_game_undo(&g) && g.active == 1 && g.turn == turn);
    sd_game_next_turn(&g);
    sd_game_next_turn(&g);
    assert(g.active == 0 && g.turn == 3);
    assert(sd_game_undo(&g) && g.active == 3 && g.turn == 2);

    /* turn state survives save/load */
    size_t n = sd_game_save(&g, buf, sizeof(buf));
    assert(sd_game_load(&h, buf, n) && h.turn == 2 && h.active == 3);
    sd_game_t bad = g;
    bad.active = 4;
    sd_game_save(&bad, buf, sizeof(buf));
    assert(!sd_game_load(&h, buf, n));

    /* defeating the active player passes the turn; one undo reverts both */
    sd_game_new(&g, &CMDR, 3);
    sd_game_next_turn(&g); /* J2 active */
    sd_game_adjust(&g, 1, SD_LIFE, 0, -40, 0);
    assert(sd_game_flush(&g) && sd_player_lost(&g, 1) && g.active == 2);
    assert(g.log[g.log_len - 1].kind == SD_TURN && g.log[g.log_len - 1].automatic);
    assert(sd_game_undo(&g) && g.active == 1 && g.p[1].life == 40 && !sd_player_lost(&g, 1));
    assert(g.log_len == 1 && g.log[0].kind == SD_TURN && !g.log[0].automatic);
    /* lethal on a non-active player: no automatic pass */
    sd_game_adjust(&g, 2, SD_LIFE, 0, -40, 0);
    sd_game_flush(&g);
    assert(g.active == 1 && g.log[g.log_len - 1].kind == SD_LIFE);
    /* game over: no pass */
    sd_game_adjust(&g, 1, SD_LIFE, 0, -40, 0);
    sd_game_flush(&g);
    assert(sd_game_over(&g) && g.log[g.log_len - 1].kind == SD_LIFE);

    /* SD_TURN is not a pendable delta */
    assert(!sd_game_adjust(&g, 0, SD_TURN, 0, 1, 0) && !g.has_pending);

    /* solo: every pass is a new turn */
    sd_game_new(&g, &CMDR, 1);
    assert(sd_game_next_turn(&g) && g.turn == 2 && g.active == 0);
}

static void test_toggle_cmd(void)
{
    sd_game_t g;
    sd_game_new(&g, &CMDR, 4);
    assert(!sd_game_pending_toggle_cmd(&g));
    sd_game_adjust(&g, 1, SD_LIFE, 1, -2, 0); /* own change: not an attack */
    assert(!sd_game_pending_toggle_cmd(&g));
    sd_game_flush(&g);
    sd_game_adjust(&g, 1, SD_LIFE, 3, -4, 0); /* J4 attacks J2 */
    assert(sd_game_pending_toggle_cmd(&g) && g.pending.kind == SD_CMD && g.pending.amount == 4);
    assert(sd_game_view(&g, 1).life == 34 && sd_game_view(&g, 1).cmd[3][0] == 4);
    assert(sd_game_pending_toggle_cmd(&g) && g.pending.kind == SD_LIFE && g.pending.amount == -4);
    sd_game_pending_toggle_cmd(&g);
    sd_game_flush(&g);
    assert(g.p[1].cmd[3][0] == 4 && g.p[1].life == 34);

    sd_rules_t std = {.start_life = 20, .cmd_lethal = 0, .poison_lethal = 10};
    sd_game_new(&g, &std, 2);
    sd_game_adjust(&g, 1, SD_LIFE, 0, -3, 0);
    assert(!sd_game_pending_toggle_cmd(&g)); /* commander damage disabled */
}

/* plan #6: six players, partner commanders, counters, table states, one new save layout */
static void test_six_players_partner(void)
{
    sd_game_t g;
    sd_game_new(&g, &CMDR, 9); /* clamped */
    assert(g.n_players == 6 && g.p[5].life == 40);
    for (int i = 0; i < 5; i++)
        sd_game_next_turn(&g);
    assert(g.active == 5 && g.turn == 1);
    sd_game_next_turn(&g);
    assert(g.active == 0 && g.turn == 2);
    assert(sd_game_adjust(&g, 5, SD_LIFE, 4, -3, 0) == false && sd_game_flush(&g) && g.p[5].life == 37);

    /* two commanders of one player are counted apart: 20 + 20 does not kill, 21 from either does */
    sd_game_adjust(&g, 1, SD_CMD, 5, 20, 0);
    sd_game_flush(&g);
    sd_game_adjust(&g, 1, SD_LIFE, 1, 60, 0);
    sd_game_flush(&g);
    sd_game_adjust(&g, 1, SD_CMD2, 5, 20, 0);
    sd_game_flush(&g);
    assert(g.p[1].cmd[5][0] == 20 && g.p[1].cmd[5][1] == 20 && g.p[1].life == 60 && !sd_player_lost(&g, 1));
    sd_game_adjust(&g, 1, SD_CMD2, 5, 1, 0);
    assert(sd_game_view(&g, 1).cmd[5][1] == 21 && sd_game_view(&g, 1).life == 59);
    sd_game_flush(&g);
    assert(sd_player_lost(&g, 1));
    assert(sd_game_undo(&g) && g.p[1].cmd[5][1] == 20 && g.p[1].life == 60 && !sd_player_lost(&g, 1));

    /* an attack turns into commander damage; a partner's damage turns back into combat damage */
    sd_game_adjust(&g, 2, SD_CMD2, 3, 4, 0);
    assert(sd_game_pending_toggle_cmd(&g) && g.pending.kind == SD_LIFE && g.pending.amount == -4);
}

static void test_counters(void)
{
    sd_game_t g;
    sd_game_new(&g, &CMDR, 2);
    assert(SD_N_COUNTERS == 7);
    for (int k = SD_POISON; k < SD_ASCEND; k++) { /* each counter is its own, 0..99 */
        sd_game_adjust(&g, 1, (sd_kind_t)k, 1, (int16_t)(k + 200), 0);
        sd_game_flush(&g);
        assert(sd_counter(&g.p[1], (sd_kind_t)k) == SD_COUNTER_MAX && g.log[g.log_len - 1].amount == SD_COUNTER_MAX);
        sd_game_adjust(&g, 1, (sd_kind_t)k, 1, (int16_t)-k, 0);
        sd_game_flush(&g);
        assert(sd_counter(&g.p[1], (sd_kind_t)k) == SD_COUNTER_MAX - k);
        assert(sd_game_undo(&g) && sd_counter(&g.p[1], (sd_kind_t)k) == SD_COUNTER_MAX);
        assert(sd_game_undo(&g) && sd_counter(&g.p[1], (sd_kind_t)k) == 0);
    }
    assert(g.log_len == 0 && g.p[1].life == 40 && sd_counter(&g.p[0], SD_ENERGY) == 0);
    /* only poison is lethal */
    sd_game_adjust(&g, 0, SD_RAD, 0, 99, 0);
    sd_game_flush(&g);
    sd_game_adjust(&g, 0, SD_ENERGY, 0, 50, 0);
    assert(sd_counter(&(sd_player_t){0}, SD_ENERGY) == 0);
    sd_player_t v = sd_game_view(&g, 0);
    assert(sd_counter(&v, SD_ENERGY) == 50 && sd_counter(&g.p[0], SD_ENERGY) == 0); /* pending shows in the view */
    sd_game_flush(&g);
    assert(!sd_player_lost(&g, 0) && !sd_game_over(&g));
    sd_game_adjust(&g, 0, SD_POISON, 0, 10, 0);
    sd_game_flush(&g);
    assert(sd_player_lost(&g, 0));
    /* states are not adjusted, they are set */
    assert(!sd_game_adjust(&g, 0, SD_ASCEND, 0, 1, 0) && !sd_game_adjust(&g, 0, SD_MONARCH, 0, 1, 0));
    assert(!sd_game_adjust(&g, 0, SD_DAYNIGHT, 0, 1, 0) && !g.has_pending);
}

static void test_states(void)
{
    sd_game_t g, h;
    unsigned char buf[8192];
    sd_game_new(&g, &CMDR, 4);
    assert(g.monarch == SD_NOBODY && g.initiative == SD_NOBODY && g.daynight == SD_DN_NONE && !g.p[2].ascended);

    /* one holder at a time; taking it from someone is one undoable step */
    assert(sd_game_set(&g, SD_MONARCH, 1) && g.monarch == 1);
    assert(!sd_game_set(&g, SD_MONARCH, 1));               /* no change, nothing logged */
    assert(!sd_game_set(&g, SD_MONARCH, 4) && g.monarch == 1); /* no such player */
    assert(sd_game_set(&g, SD_MONARCH, 3) && g.monarch == 3 && g.log_len == 2);
    assert(sd_game_set(&g, SD_INITIATIVE, 3) && g.initiative == 3 && g.monarch == 3); /* independent */
    assert(sd_game_undo(&g) && g.initiative == SD_NOBODY);
    assert(sd_game_undo(&g) && g.monarch == 1);
    assert(sd_game_set(&g, SD_MONARCH, SD_NOBODY) && g.monarch == SD_NOBODY);
    assert(sd_game_undo(&g) && g.monarch == 1);

    assert(sd_game_set(&g, SD_DAYNIGHT, SD_DN_DAY) && sd_game_set(&g, SD_DAYNIGHT, SD_DN_NIGHT));
    assert(!sd_game_set(&g, SD_DAYNIGHT, 3) && g.daynight == SD_DN_NIGHT);
    assert(sd_game_undo(&g) && g.daynight == SD_DN_DAY);

    assert(sd_game_set(&g, SD_ASCEND, 2) && g.p[2].ascended && !g.p[1].ascended);
    assert(sd_game_set(&g, SD_ASCEND, 2) && !g.p[2].ascended); /* toggles */
    assert(sd_game_undo(&g) && g.p[2].ascended);
    assert(!sd_game_set(&g, SD_ASCEND, 4) && !sd_game_set(&g, SD_LIFE, 0) && !sd_game_set(&g, SD_TURN, 0));

    /* a set commits the pending delta first, and the log keeps the order */
    sd_game_adjust(&g, 0, SD_LIFE, 0, -2, 0);
    assert(sd_game_set(&g, SD_INITIATIVE, 0) && !g.has_pending && g.p[0].life == 38);
    assert(g.log[g.log_len - 2].kind == SD_LIFE && g.log[g.log_len - 1].kind == SD_INITIATIVE);
    /* the holder being defeated moves nothing by itself */
    sd_game_adjust(&g, 1, SD_LIFE, 0, -40, 0);
    sd_game_flush(&g);
    assert(sd_player_lost(&g, 1) && g.monarch == 1);

    /* everything survives save/load, the format name included */
    snprintf(g.format, sizeof(g.format), "Commander 2HG");
    sd_game_adjust(&g, 3, SD_TREASURE, 3, 5, 0);
    sd_game_flush(&g);
    sd_game_adjust(&g, 3, SD_CMD2, 0, 6, 0);
    sd_game_flush(&g);
    size_t n = sd_game_save(&g, buf, sizeof(buf));
    assert(n && sd_game_load(&h, buf, n));
    assert(h.monarch == 1 && h.initiative == 0 && h.daynight == SD_DN_DAY && h.p[2].ascended);
    assert(sd_counter(&h.p[3], SD_TREASURE) == 5 && h.p[3].cmd[0][1] == 6 && h.p[3].life == 34);
    assert(!strcmp(h.format, "Commander 2HG") && h.log_len == g.log_len);
    assert(sd_game_undo(&h) && h.p[3].cmd[0][1] == 0 && h.p[3].life == 40);

    /* a save is refused when a state or a logged state is out of range, or the name is not terminated */
    sd_game_t bad = g;
    bad.monarch = 4;
    sd_game_save(&bad, buf, sizeof(buf));
    assert(!sd_game_load(&h, buf, n));
    bad = g;
    bad.daynight = 3;
    sd_game_save(&bad, buf, sizeof(buf));
    assert(!sd_game_load(&h, buf, n));
    bad = g;
    bad.log[0].src = 7; /* log[0] is a monarch change: 7 is neither a player nor SD_NOBODY */
    assert(g.log[0].kind == SD_MONARCH && g.log[0].src == SD_NOBODY);
    sd_game_save(&bad, buf, sizeof(buf));
    assert(!sd_game_load(&h, buf, n));
    bad = g;
    bad.log[1].kind = SD_KIND_COUNT;
    sd_game_save(&bad, buf, sizeof(buf));
    assert(!sd_game_load(&h, buf, n));
    bad = g;
    memset(bad.format, 'x', sizeof(bad.format));
    sd_game_save(&bad, buf, sizeof(buf));
    assert(!sd_game_load(&h, buf, n));

    /* a save of a previous layout ("SDG3") is not migrated: refused, whatever its size */
    sd_game_save(&g, buf, sizeof(buf));
    buf[0] = '3'; /* little-endian magic: 'SDG4' is stored 4 G D S */
    assert(!sd_game_load(&h, buf, n) && !sd_game_load(&h, buf, 1412));
    buf[0] = '4';
    assert(sd_game_load(&h, buf, n));
}

static void test_rule_options(void)
{
    sd_game_t g;
    sd_rules_t r = CMDR;

    /* counter-clockwise: J1 -> J4 -> J3 -> J2 -> J1, the turn number moves when J1 is back */
    r.opts = SD_OPT_COUNTER_CW;
    sd_game_new(&g, &r, 4);
    static const uint8_t order[] = {3, 2, 1, 0, 3};
    static const uint16_t turn[] = {1, 1, 1, 2, 2};
    for (int i = 0; i < 5; i++) {
        assert(sd_game_next_turn(&g) && g.active == order[i] && g.turn == turn[i]);
    }
    sd_game_adjust(&g, 2, SD_LIFE, 2, -40, 0); /* J3 out: J4 -> J2 skips it */
    sd_game_flush(&g);
    assert(sd_game_next_turn(&g) && g.active == 1 && g.turn == 2);
    assert(sd_game_undo(&g) && g.active == 3 && g.turn == 2);

    /* commander damage counted apart: the tally moves, the life total does not; still lethal at the threshold */
    r.opts = SD_OPT_CMD_APART;
    sd_game_new(&g, &r, 2);
    sd_game_adjust(&g, 0, SD_CMD, 1, 20, 0);
    assert(sd_game_view(&g, 0).life == 40 && sd_game_view(&g, 0).cmd[1][0] == 20);
    sd_game_flush(&g);
    assert(g.p[0].life == 40 && g.p[0].cmd[1][0] == 20 && !sd_player_lost(&g, 0));
    sd_game_adjust(&g, 0, SD_CMD, 1, 1, 0);
    sd_game_flush(&g);
    assert(g.p[0].life == 40 && sd_player_lost(&g, 0) && sd_game_over(&g));
    assert(sd_game_undo(&g) && sd_game_undo(&g) && g.p[0].life == 40 && g.p[0].cmd[1][0] == 0);

    /* manual KO: nobody is defeated, so nothing ends and no turn is skipped */
    r.opts = SD_OPT_MANUAL_KO;
    sd_game_new(&g, &r, 2);
    sd_game_adjust(&g, 0, SD_LIFE, 0, -45, 0);
    sd_game_flush(&g);
    assert(g.p[0].life == -5 && !sd_player_lost(&g, 0) && !sd_game_over(&g) && g.active == 0 && g.log_len == 1);

    /* manual win: players are defeated and skipped, the game goes on even with one left */
    r.opts = SD_OPT_MANUAL_WIN;
    sd_game_new(&g, &r, 2);
    sd_game_adjust(&g, 0, SD_LIFE, 0, -40, 0);
    sd_game_flush(&g);
    assert(sd_player_lost(&g, 0) && !sd_game_over(&g) && sd_game_winner(&g) == 1 && g.active == 1);

    /* commit delay from the rules; 0 keeps SD_COMMIT_MS */
    r.opts = 0;
    r.commit_ms = 1500;
    sd_game_new(&g, &r, 1);
    sd_game_adjust(&g, 0, SD_LIFE, 0, -1, 1000);
    assert(!sd_game_tick(&g, 2499) && sd_game_tick(&g, 2500));
    assert(sd_commit_ms(&g) == 1500);

    /* options survive a save; unknown option bits are refused */
    uint8_t buf[sizeof(sd_game_t) + 16];
    sd_game_t h;
    r.opts = SD_OPT_COUNTER_CW | SD_OPT_CMD_APART;
    sd_game_new(&g, &r, 3);
    assert(sd_game_save(&g, buf, sizeof(buf)) && sd_game_load(&h, buf, sd_game_save_size()));
    assert(h.rules.opts == r.opts && h.rules.commit_ms == 1500);
    g.rules.opts = SD_OPT_ALL + 1;
    sd_game_save(&g, buf, sizeof(buf));
    assert(!sd_game_load(&h, buf, sd_game_save_size()));
}

static void test_settings(void)
{
    sd_settings_t s, l;
    uint8_t buf[256];
    sd_settings_defaults(&s);
    for (int i = 0; i < SD_S_COUNT; i++) { /* every setting has bounds and a default inside them */
        assert(sd_setting_min(i) < sd_setting_max(i));
        assert(s.v[i] >= sd_setting_min(i) && s.v[i] <= sd_setting_max(i));
    }
    assert(s.v[SD_S_CLICKS] == 1 && s.v[SD_S_AUTO_KO] && s.v[SD_S_COMMIT_MS] == SD_COMMIT_MS && !s.pin[0]);

    /* clamped on set */
    sd_setting_set(&s, SD_S_CLICKS, 40);
    assert(s.v[SD_S_CLICKS] == 12);
    sd_setting_set(&s, SD_S_CLICKS, -3);
    assert(s.v[SD_S_CLICKS] == 1);

    /* round trip */
    sd_setting_set(&s, SD_S_START_LIFE, 30);
    sd_setting_set(&s, SD_S_HAPTIC, 1);
    strcpy(s.name, "DRAGON ROUGE");
    strcpy(s.owner_msg, "A JU");
    strcpy(s.pin, "0429");
    size_t n = sd_settings_save(&s, buf, sizeof(buf));
    assert(n == sd_settings_save_size() && n <= sizeof(buf) && !sd_settings_save(&s, buf, n - 1));
    assert(n <= sizeof(sd_settings_t) + 16); /* the firmware's NVS buffer (main/main.c) */
    assert(sd_settings_load(&l, buf, n) && !memcmp(&l, &s, sizeof(s)));

    /* a save from an older firmware (two settings fewer): they take their defaults, the rest is kept */
    uint16_t fewer = SD_S_COUNT - 2;
    uint8_t old[256];
    memcpy(old, buf, n);
    memcpy(old + 4, &fewer, sizeof(fewer)); /* header: magic, then the count */
    assert(sd_settings_load(&l, old, n - 2 * sizeof(int16_t)));
    assert(l.v[SD_S_START_LIFE] == 30 && !strcmp(l.name, "DRAGON ROUGE") && l.v[SD_S_HAPTIC] == 3 && !l.v[SD_S_SETUP_DONE]);

    /* out-of-range values are clamped, a malformed PIN is dropped, garbage gives the defaults */
    int16_t big = 30000;
    memcpy(old, buf, n);
    memcpy(old + n - sizeof(s.v) + SD_S_CLICKS * sizeof(int16_t), &big, sizeof(big));
    memcpy(old + 4 + 2 + sizeof(s.name) + sizeof(s.owner_msg), "12a4", 5);
    assert(sd_settings_load(&l, old, n) && l.v[SD_S_CLICKS] == 12 && !l.pin[0]);
    buf[0] ^= 0xFF;
    assert(!sd_settings_load(&l, buf, n) && l.v[SD_S_CLICKS] == 1 && !l.name[0]);
    buf[0] ^= 0xFF;
    assert(!sd_settings_load(&l, buf, n - 1) && !sd_settings_load(&l, buf, 3));
    assert(sd_pin_valid("") && sd_pin_valid("0000") && !sd_pin_valid("123") && !sd_pin_valid("12345") && !sd_pin_valid("12 4"));

    /* settings -> rules of a new game */
    sd_rules_t r = CMDR, std = {.start_life = 20, .poison_lethal = 10};
    sd_settings_defaults(&s);
    sd_settings_rules(&s, &r);
    assert(r.start_life == 40 && r.cmd_lethal == 21 && r.poison_lethal == 10 && r.opts == 0 && r.commit_ms == SD_COMMIT_MS);
    sd_setting_set(&s, SD_S_START_LIFE, 30);
    sd_setting_set(&s, SD_S_CMD_LETHAL, 16);
    sd_setting_set(&s, SD_S_POISON_LETHAL, 15);
    sd_setting_set(&s, SD_S_AUTO_KO, 0);
    sd_setting_set(&s, SD_S_AUTO_WIN, 0);
    sd_setting_set(&s, SD_S_CMD_LIFE, 0);
    sd_setting_set(&s, SD_S_TURN_ORDER, 1);
    sd_setting_set(&s, SD_S_COMMIT_MS, 1200);
    sd_settings_rules(&s, &r);
    assert(r.start_life == 30 && r.cmd_lethal == 16 && r.poison_lethal == 15 && r.opts == SD_OPT_ALL && r.commit_ms == 1200);
    sd_settings_rules(&s, &std); /* a format without commander damage stays without it */
    assert(std.start_life == 30 && std.cmd_lethal == 0 && std.poison_lethal == 15);

    /* clicks per point: 3 detents make one point, the remainder is kept, both ways */
    int16_t rest = 0;
    assert(sd_clicks_to_points(&rest, 1, 3) == 0 && sd_clicks_to_points(&rest, 1, 3) == 0);
    assert(sd_clicks_to_points(&rest, 1, 3) == 1 && rest == 0);
    assert(sd_clicks_to_points(&rest, 7, 3) == 2 && rest == 1);
    assert(sd_clicks_to_points(&rest, -4, 3) == -1 && rest == 0);
    assert(sd_clicks_to_points(&rest, -5, 1) == -5 && rest == 0);
}

int main(void)
{
    test_delta_commit();
    test_undo();
    test_defeats();
    test_save_load();
    test_turns();
    test_toggle_cmd();
    test_six_players_partner();
    test_counters();
    test_states();
    test_rule_options();
    test_settings();
    puts("core: all tests passed");
    return 0;
}
