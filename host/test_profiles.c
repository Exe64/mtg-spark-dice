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
