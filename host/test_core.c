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

int main(void)
{
    test_delta_commit();
    test_undo();
    test_defeats();
    test_save_load();
    puts("core: all tests passed");
    return 0;
}
