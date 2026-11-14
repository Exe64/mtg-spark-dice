/* What the device does at power-on, one scenario per process (the UI starts once):
 *   test_boot <profiles_dir> <scenario> */
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "shot.h"
#include "ui.h"

static uint32_t t = 1000;

/* through the logo */
static void boot(void)
{
    assert(ui_screen() == UI_SCR_BOOT);
    ui_tick(t);
    t += UI_BOOT_MS;
    ui_tick(t);
}

int main(int argc, char **argv)
{
    assert(argc == 3);
    const char *sc = argv[2];
    static sd_profile_t prof[SD_PROFILES_MAX];
    int n_prof = sd_profiles_load(argv[1], prof, SD_PROFILES_MAX);
    assert(n_prof == 10);
    shot_init();

    sd_settings_t st;
    sd_settings_defaults(&st);
    st.v[SD_S_SETUP_DONE] = 1;
    static sd_game_t g; /* a saved game: Commander at three, J2 at 12 */
    sd_game_new(&g, &prof[0].rules, 3);
    strcpy(g.format, prof[0].name);
    g.p[1].life = 12;

    if (!strcmp(sc, "start-on-game")) { /* straight back into the saved game */
        st.v[SD_S_START_ON] = 1;
        ui_set_settings(&st);
        ui_init(NULL, prof, n_prof, &g);
        boot();
        assert(ui_screen() == UI_SCR_GAME && ui_game()->n_players == 3 && ui_game()->p[1].life == 12);
    } else if (!strcmp(sc, "start-on-game-none")) { /* nothing saved: home */
        st.v[SD_S_START_ON] = 1;
        ui_set_settings(&st);
        ui_init(NULL, prof, n_prof, NULL);
        boot();
        assert(ui_screen() == UI_SCR_HOME);
    } else if (!strcmp(sc, "start-on-home")) { /* the default: home, REPRENDRE offered */
        ui_set_settings(&st);
        ui_init(NULL, prof, n_prof, &g);
        boot();
        assert(ui_screen() == UI_SCR_HOME);
        ui_tap(180, 300, t);
        assert(ui_screen() == UI_SCR_GAME && ui_game()->p[1].life == 12);
    } else {
        fprintf(stderr, "unknown scenario %s\n", sc);
        return 1;
    }
    printf("boot %s: ok\n", sc);
    return 0;
}
