/* UI flow test + screenshots: drives the UI with scripted input, asserts state, writes one PNG per screen.
 *   shots <profiles_dir> <out_dir> [booster]     (default theme otherwise) */
#undef NDEBUG
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "shot.h"
#include "theme.h"
#include "ui.h"

static const char *out_dir;
static int saves, haptics[3];
static sd_game_t saved;

static void on_save(const sd_game_t *g)
{
    saves++;
    saved = *g;
}
static void on_haptic(ui_haptic_t p) { haptics[p]++; }

static uint32_t t = 1000;

/* WCAG relative luminance and contrast ratio */
static double lum(uint32_t c)
{
    double l = 0;
    static const double w[] = {0.2126, 0.7152, 0.0722};
    for (int i = 0; i < 3; i++) {
        double v = ((c >> (16 - 8 * i)) & 0xFF) / 255.0;
        l += w[i] * (v <= 0.03928 ? v / 12.92 : pow((v + 0.055) / 1.055, 2.4));
    }
    return l;
}
static double contrast(uint32_t a, uint32_t b)
{
    double la = lum(a), lb = lum(b);
    return (fmax(la, lb) + 0.05) / (fmin(la, lb) + 0.05);
}
static bool near(uint32_t a, uint32_t b, int tol)
{
    for (int s = 0; s <= 16; s += 8)
        if (abs((int)((a >> s) & 0xFF) - (int)((b >> s) & 0xFF)) > tol)
            return false;
    return true;
}

static void snap(const char *name)
{
    char path[512];
    shot_set_time(t);
    shot_render();
    snprintf(path, sizeof(path), "%s/%s.png", out_dir, name);
    assert(shot_save(path));
    printf("  %s\n", path);
}

/* seat centres for n players (J1 bottom, clockwise), matching ui.c geometry */
static void seat_xy(int n, int i, int *x, int *y)
{
    static const int r[] = {0, 0, 96, 104, 108};
    double a = (90.0 + i * 360.0 / n) * 3.14159265 / 180.0;
    *x = 180 + (int)(r[n] * cos(a));
    *y = 180 + (int)(r[n] * sin(a));
}

int main(int argc, char **argv)
{
    assert(argc == 3 || argc == 4);
    out_dir = argv[2];
    char path[512];
    sd_profile_t prof[2];
    const char *err = NULL;
    snprintf(path, sizeof(path), "%s/commander.json", argv[1]);
    assert(sd_profile_load(path, &prof[0], &err));
    snprintf(path, sizeof(path), "%s/standard.json", argv[1]);
    assert(sd_profile_load(path, &prof[1], &err));

    shot_init();
    ui_hooks_t hooks = {.haptic = on_haptic, .save = on_save};
    if (argc == 4)
        ui_set_theme(UI_THEME_BOOSTER, 0);
    ui_init(&hooks, prof, 2, NULL);

    /* --- home: Commander, 4 players */
    assert(ui_screen() == UI_SCR_HOME);
    snap("01-accueil");

    /* --- solo: 1 player via knob on the players row */
    ui_tap(180, 200, t);      /* focus players row */
    ui_knob(-3, t);           /* 4 -> 1 */
    snap("02-accueil-1-joueur");
    ui_tap(180, 280, t);      /* JOUER */
    assert(ui_screen() == UI_SCR_GAME && ui_game()->n_players == 1 && ui_game()->p[0].life == 40);
    assert(saves == 1);
    snap("03-solo");
    ui_knob(-1, t);
    ui_knob(-1, t + 100);
    ui_knob(-1, t + 200);
    t += 200 + 400;           /* 400 ms into the 800 ms seal */
    ui_tick(t);
    assert(ui_game()->has_pending && ui_game()->p[0].life == 40);
    snap("04-solo-delta-en-attente");
    if (argc == 3) { /* luminous: half-way through the delay the light arc runs from 12 o'clock to 6 o'clock */
        assert(near(shot_pixel(180 + 120, 180 - 120), ui_theme_lumineux.ink, 40));
        assert(!near(shot_pixel(180 - 120, 180 - 120), ui_theme_lumineux.ink, 40));
    }
    t += 400;
    ui_tick(t);
    assert(!ui_game()->has_pending && ui_game()->p[0].life == 37 && saves == 2);
    snap("05-solo-valide");

    /* --- menu, undo */
    ui_long_press(180, 180, t);
    assert(ui_screen() == UI_SCR_MENU);
    snap("06-menu");
    ui_tap(180, 76, t);       /* ANNULER */
    assert(ui_screen() == UI_SCR_GAME && ui_game()->p[0].life == 40 && ui_game()->log_len == 0);

    /* --- new game from menu keeps the old one resumable; 4-player Commander arena */
    ui_long_press(180, 180, t);
    ui_tap(180, 232, t);      /* NOUVELLE PARTIE */
    assert(ui_screen() == UI_SCR_HOME);
    snap("07-accueil-reprendre");
    ui_tap(180, 200, t);      /* focus players row (home resets focus to profile) */
    ui_knob(3, t);            /* 1 -> 4 */
    ui_tap(100, 280, t);      /* JOUER (left button) */
    assert(ui_screen() == UI_SCR_GAME && ui_game()->n_players == 4);
    snap("08-arene-4");

    /* turns: a hub tap passes the turn clockwise; the undo pill (4 s) takes it back */
    assert(ui_game()->turn == 1 && ui_game()->active == 0);
    ui_tap(180, 180, t);
    assert(ui_game()->active == 1);
    snap("08b-arene-pastille-annuler");
    ui_tap(180, 180, t);      /* pill: undo the pass */
    assert(ui_game()->active == 0);
    ui_tap(180, 180, t);      /* pass again */
    assert(ui_game()->active == 1 && ui_game()->turn == 1);
    t += 4100;
    ui_tick(t);
    snap("08c-arene-tour-j2");

    int x, y, x2, y2;
    /* J2 loses 6 */
    seat_xy(4, 1, &x, &y);
    ui_tap(x, y, t);
    assert(ui_selected() == 1);
    ui_tap(x, y + 50, t);     /* "+" side of J2 (seat turned 90°: its right is screen-down) */
    assert(ui_game()->has_pending && ui_game()->pending.amount == 1);
    ui_knob(-7, t);
    assert(ui_game()->pending.amount == -6);
    t += 300;
    ui_tick(t);
    snap("09-arene-delta-en-attente");
    t += 600;
    ui_tick(t);
    assert(ui_game()->p[1].life == 34);

    /* attack: drag J3 -> J1 = combat damage; a hub tap turns it into commander damage; ccw adds 4 more */
    t += 4100;
    ui_tick(t);
    seat_xy(4, 2, &x, &y);
    seat_xy(4, 0, &x2, &y2);
    ui_drag(x, y, x2, y2, t);
    assert(ui_selected() == 0 && ui_game()->pending.kind == SD_LIFE && ui_game()->pending.src == 2 &&
           ui_game()->pending.amount == -1);
    ui_knob(-2, t + 20);
    t += 300;
    ui_tick(t);
    snap("10a-arene-attaque");
    ui_knob(2, t);
    ui_tap(180, 180, t + 10);
    ui_knob(-4, t + 50);
    t += 450;
    ui_tick(t);
    assert(ui_game()->pending.kind == SD_CMD && ui_game()->pending.amount == 5);
    snap("10-arene-degats-commandant");
    t += 500;
    ui_tick(t);
    assert(ui_game()->p[0].cmd[2] == 5 && ui_game()->p[0].life == 35);

    /* poison on J4 through its sheet */
    seat_xy(4, 3, &x, &y);
    ui_tap(x, y, t);
    ui_long_press(x, y, t);
    ui_tap(180, 180, t);      /* FICHE J4 */
    assert(ui_screen() == UI_SCR_SHEET);
    ui_knob(3, t);
    t += 200;
    ui_tick(t);
    snap("11-fiche-joueur");
    t += 700;
    ui_tick(t);
    assert(ui_game()->p[3].poison == 3);
    ui_tap(180, 180, t);
    assert(ui_screen() == UI_SCR_GAME);

    /* log */
    ui_long_press(180, 180, t);
    ui_tap(180, 128, t);
    assert(ui_screen() == UI_SCR_LOG);
    snap("12-journal");
    ui_tap(180, 180, t);
    ui_tap(180, 284, t);      /* RETOUR */
    assert(ui_screen() == UI_SCR_GAME);

    /* J2 out (life), J4 out (poison): J1 and J3 left, then J1 kills J3 */
    seat_xy(4, 1, &x, &y);
    ui_tap(x, y, t);
    ui_knob(-34, t);
    t += 900;
    ui_tick(t);
    assert(sd_player_lost(ui_game(), 1) && haptics[UI_HAPTIC_DEFEAT] == 1);
    assert(ui_game()->active == 2); /* J2 was active: the turn moved on to J3 */
    snap("13-arene-joueur-vaincu");
    ui_tap(180, 180, t);      /* pill: one undo restores J2 and its turn */
    assert(ui_game()->p[1].life == 34 && ui_game()->active == 1 && !sd_player_lost(ui_game(), 1));
    snap("13b-arene-annulation-coup-fatal");
    ui_knob(-34, t);          /* replay the lethal hit */
    t += 900;
    ui_tick(t);
    assert(sd_player_lost(ui_game(), 1) && ui_game()->active == 2);
    seat_xy(4, 3, &x, &y);
    ui_tap(x, y, t);
    ui_long_press(x, y, t);
    ui_tap(180, 180, t);
    ui_knob(7, t);
    ui_tap(180, 180, t);      /* leave sheet = commit */
    seat_xy(4, 2, &x, &y);
    ui_tap(x, y, t);
    ui_knob(-40, t);
    t += 900;
    ui_tick(t);
    assert(ui_screen() == UI_SCR_OVER && sd_game_winner(ui_game()) == 0);
    snap("14-fin-de-partie");

    /* undo from game over revives J3 */
    ui_tap(180, 223, t);
    assert(ui_screen() == UI_SCR_GAME && !sd_player_lost(ui_game(), 2));
    assert(saved.log_len == ui_game()->log_len); /* every change persisted */

    /* --- 2 and 3 player arenas (Standard profile, no commander damage) */
    ui_long_press(180, 180, t);
    ui_tap(180, 232, t);
    ui_tap(180, 104, t);      /* profile row already focused -> next profile */
    ui_tap(100, 280, t);
    assert(ui_game()->n_players == 2 && ui_game()->p[0].life == 20);
    seat_xy(2, 0, &x, &y);
    seat_xy(2, 1, &x2, &y2);
    ui_drag(x, y, x2, y2, t); /* J1 attacks J2 */
    assert(ui_game()->has_pending && ui_game()->pending.kind == SD_LIFE && ui_game()->pending.player == 1);
    ui_tap(180, 180, t);      /* Standard: no commander damage, the hub tap commits */
    assert(!ui_game()->has_pending && ui_game()->p[1].life == 19);
    t += 4100;
    ui_tick(t);
    snap("15-arene-2-standard");
    ui_long_press(180, 180, t);
    ui_tap(180, 232, t);
    ui_tap(180, 200, t);
    ui_knob(1, t);            /* 2 -> 3 */
    ui_tap(100, 280, t);
    assert(ui_game()->n_players == 3);
    seat_xy(3, 2, &x, &y);
    ui_tap(x, y, t);
    assert(ui_selected() == 2);
    snap("16-arene-3");

    /* --- themes and grounds, on the home screen: (180,180), (180,3) and (3,180) are bare ground there */
    ui_long_press(180, 180, t);
    ui_tap(180, 232, t);      /* NOUVELLE PARTIE */
    assert(ui_screen() == UI_SCR_HOME);
    ui_set_theme(UI_THEME_BOOSTER, 0); /* flat ground */
    shot_render();
    assert(shot_pixel(180, 180) == ui_theme_booster.ground && shot_pixel(180, 3) == ui_theme_booster.ground);
    assert(ui_ground_rim(0x18214A) == 0x060914); /* the contract's well */
    for (int gi = 0; gi < UI_GROUND_COUNT; gi++) {
        uint32_t c = ui_grounds[gi], rim = ui_ground_rim(c);
        assert(contrast(ui_theme_lumineux.ink, c) >= 7.0);
        ui_set_theme(UI_THEME_LUMINEUX, gi);
        char name[32];
        snprintf(name, sizeof(name), "17-lumineux-fond-%d", gi);
        snap(name);
        assert(near(shot_pixel(180, 180), c, 3));  /* centre of the well */
        assert(near(shot_pixel(180, 3), rim, 4));  /* rim */
        assert(near(shot_pixel(3, 180), rim, 4));  /* radial, not vertical */
    }
    ui_set_theme(99, 99);     /* out of range: default theme, default ground */
    shot_render();
    assert(UI_THEME_DEFAULT == UI_THEME_LUMINEUX && near(shot_pixel(180, 180), ui_grounds[0], 3));

    printf("shots: all flows passed (%d saves, %d detents, %d commits, %d defeats)\n", saves,
           haptics[UI_HAPTIC_DETENT], haptics[UI_HAPTIC_COMMIT], haptics[UI_HAPTIC_DEFEAT]);
    return 0;
}
