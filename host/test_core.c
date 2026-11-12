#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "game.h"

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
    assert(g.p[1].poison == 0 && g.p[1].life == 40 && !g.has_pending);
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
    assert(g.p[2].cmd[1] == 20 && g.p[2].life == 20 && !sd_player_lost(&g, 2));
    sd_game_adjust(&g, 2, SD_CMD, 3, 20, 0); /* other source: separate counter */
    sd_game_flush(&g);
    assert(g.p[2].life == 0 && sd_player_lost(&g, 2)); /* dead by life, not commander */
    sd_game_undo(&g);
    sd_game_adjust(&g, 2, SD_LIFE, 0, 30, 0);
    sd_game_flush(&g);
    sd_game_adjust(&g, 2, SD_CMD, 1, 1, 0);
    sd_game_flush(&g);
    assert(g.p[2].cmd[1] == 21 && g.p[2].life == 49 && sd_player_lost(&g, 2));
    sd_game_undo(&g);
    assert(g.p[2].cmd[1] == 20 && g.p[2].life == 50);
    assert(sd_game_view(&g, 2).cmd[1] == 20);

    /* pending commander damage shows on life */
    sd_game_adjust(&g, 2, SD_CMD, 1, 1, 0);
    assert(sd_game_view(&g, 2).life == 49 && sd_game_view(&g, 2).cmd[1] == 21);

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
    assert(h.n_players == 4 && h.p[3].cmd[0] == 7 && h.p[3].life == 33 && h.p[1].poison == 2);
    assert(h.log_len == 2 && h.rules.start_life == 40 && !h.has_pending);
    assert(sd_game_undo(&h) && h.p[1].poison == 0);

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
    assert(sd_game_view(&g, 1).life == 34 && sd_game_view(&g, 1).cmd[3] == 4);
    assert(sd_game_pending_toggle_cmd(&g) && g.pending.kind == SD_LIFE && g.pending.amount == -4);
    sd_game_pending_toggle_cmd(&g);
    sd_game_flush(&g);
    assert(g.p[1].cmd[3] == 4 && g.p[1].life == 34);

    sd_rules_t std = {.start_life = 20, .cmd_lethal = 0, .poison_lethal = 10};
    sd_game_new(&g, &std, 2);
    sd_game_adjust(&g, 1, SD_LIFE, 0, -3, 0);
    assert(!sd_game_pending_toggle_cmd(&g)); /* commander damage disabled */
}

int main(void)
{
    test_delta_commit();
    test_undo();
    test_defeats();
    test_save_load();
    test_turns();
    test_toggle_cmd();
    puts("core: all tests passed");
    return 0;
}
