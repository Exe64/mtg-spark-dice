/* Render cost of each theme on the PC: average time of a full 360x360 redraw per screen.
 *   bench <profiles_dir>
 * The absolute numbers are this PC's; the lumineux/booster ratio is what carries over to the ESP32-S3. */
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <time.h>

#include "lvgl.h"
#include "shot.h"
#include "theme.h"
#include "ui.h"

#define FRAMES 200

static double frame_ms(void)
{
    struct timespec a, b;
    clock_gettime(CLOCK_MONOTONIC, &a);
    for (int i = 0; i < FRAMES; i++)
        shot_render();
    clock_gettime(CLOCK_MONOTONIC, &b);
    return ((b.tv_sec - a.tv_sec) * 1e3 + (b.tv_nsec - a.tv_nsec) / 1e6) / FRAMES;
}

static void row(const char *screen)
{
    ui_set_theme(UI_THEME_BOOSTER, 0);
    double booster = frame_ms();
    ui_set_theme(UI_THEME_LUMINEUX, 0);
    double lumineux = frame_ms();
    printf("%-22s %8.2f %8.2f %7.1fx\n", screen, booster, lumineux, lumineux / booster);
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    char path[512];
    sd_profile_t prof[1];
    const char *err = NULL;
    snprintf(path, sizeof(path), "%s/commander.json", argv[1]);
    if (!sd_profile_load(path, &prof[0], &err))
        return 1;
    shot_init();
    ui_init(NULL, prof, 1, NULL);
    uint32_t t = 1000;
    printf("%-22s %8s %8s %8s   (ms per full redraw, %d frames)\n", "screen", "booster", "lumineux", "ratio", FRAMES);
    row("boot");
    ui_tap(180, 180, t); /* skip the logo */
    row("home");
    ui_tap(180, 280, t); /* JOUER: Commander, 4 players */
    row("arena 4, at rest");
    ui_knob(-3, t);
    row("arena 4, pending");
    ui_long_press(180, 180, t);
    row("menu");
    ui_tap(180, 232, t); /* NOUVELLE PARTIE -> home */
    ui_tap(180, 200, t); /* players row */
    ui_knob(-3, t);      /* 4 -> 1 */
    ui_tap(100, 280, t); /* JOUER */
    row("solo, at rest");
    ui_knob(-3, t);
    row("solo, pending");
    return 0;
}
