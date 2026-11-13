#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "profile.h"

static void reject(const char *json, const char *expected_err)
{
    sd_profile_t p;
    const char *err = NULL;
    assert(!sd_profile_parse(json, &p, &err));
    if (!strstr(err, expected_err)) {
        fprintf(stderr, "json %s: got '%s', expected '%s'\n", json, err, expected_err);
        assert(0);
    }
}

int main(void)
{
    sd_profile_t p;
    const char *err = NULL;

    assert(sd_profile_load(PROFILES_DIR "/commander.json", &p, &err));
    assert(!strcmp(p.name, "Commander") && p.players == 4);
    assert(p.rules.start_life == 40 && p.rules.cmd_lethal == 21 && p.rules.poison_lethal == 10);

    assert(sd_profile_load(PROFILES_DIR "/standard.json", &p, &err));
    assert(!strcmp(p.name, "Standard") && p.players == 2);
    assert(p.rules.start_life == 20 && p.rules.cmd_lethal == 0 && p.rules.poison_lethal == 10);

    /* profile feeds the core directly */
    sd_game_t g;
    sd_game_new(&g, &p.rules, p.players);
    assert(g.n_players == 2 && g.p[1].life == 20);

    assert(!sd_profile_load(PROFILES_DIR "/missing.json", &p, &err) && strstr(err, "open"));

    const char *ok = "{\"name\":\"X\",\"start_life\":30,\"players\":3,\"commander_damage\":0,\"poison\":0,\"extra\":1}";
    assert(sd_profile_parse(ok, &p, NULL) && p.players == 3);

    /* table size: optional, 2..4 by default; a fixed size is min == max */
    assert(p.players_min == 2 && p.players_max == 4 && !p.family[0]);
    reject("{\"name\":\"X\",\"start_life\":40,\"players\":2,\"players_min\":3,\"players_max\":2,\"commander_damage\":0,\"poison\":0}", "players_min");
    reject("{\"name\":\"X\",\"start_life\":40,\"players\":2,\"players_max\":9,\"commander_damage\":0,\"poison\":0}", "players_min");
    reject("{\"name\":\"X\",\"family\":\"abcdefghijklmnopqrstuvwxyz\",\"start_life\":40,\"players\":2}", "family");

    /* the shipped formats: singleton first, then constructed, which is always a 20-life duel without commander damage */
    static sd_profile_t all[SD_PROFILES_MAX];
    int n = sd_profiles_load(PROFILES_DIR, all, SD_PROFILES_MAX);
    assert(n == 10 && !strcmp(all[0].name, "Commander") && !strcmp(all[1].name, "Commander 1v1"));
    assert(all[1].rules.start_life == 30 && all[1].rules.cmd_lethal == 21 && all[1].players_max == 2);
    assert(all[2].rules.start_life == 60 && all[2].rules.poison_lethal == 15);
    assert(all[3].rules.start_life == 20 && all[3].rules.cmd_lethal == 0 && !strcmp(all[3].family, "Singleton"));
    for (int i = 4; i < n; i++)
        assert(!strcmp(all[i].family, "Construit") && all[i].rules.start_life == 20 && !all[i].rules.cmd_lethal &&
               all[i].rules.poison_lethal == 10 && all[i].players_min == 2 && all[i].players_max == 2);

    reject("not json", "invalid JSON");
    reject("[1,2]", "invalid JSON");
    reject("{\"start_life\":40}", "name");
    reject("{\"name\":\"\",\"start_life\":40}", "name");
    reject("{\"name\":\"abcdefghijklmnopqrstuvwxyz\"}", "name");
    reject("{\"name\":\"X\",\"start_life\":0,\"players\":2,\"commander_damage\":0,\"poison\":0}", "start_life");
    reject("{\"name\":\"X\",\"start_life\":40.5,\"players\":2,\"commander_damage\":0,\"poison\":0}", "start_life");
    reject("{\"name\":\"X\",\"start_life\":\"40\",\"players\":2,\"commander_damage\":0,\"poison\":0}", "start_life");
    reject("{\"name\":\"X\",\"start_life\":40,\"players\":5,\"commander_damage\":0,\"poison\":0}", "players");
    reject("{\"name\":\"X\",\"start_life\":40,\"players\":2,\"commander_damage\":-1,\"poison\":0}", "commander_damage");
    reject("{\"name\":\"X\",\"start_life\":40,\"players\":2,\"commander_damage\":0}", "poison");

    puts("profiles: all tests passed");
    return 0;
}
