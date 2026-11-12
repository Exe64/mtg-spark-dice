/* Smoke test of the headless renderer: known scene -> pixels + valid 360x360 PNG with round mask. */
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "shot.h"

int main(int argc, char **argv)
{
    const char *out = argc > 1 ? argv[1] : "test_shot.png";
    shot_init();
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x123456), 0);
    lv_obj_t *box = lv_obj_create(scr);
    lv_obj_set_size(box, 60, 60);
    lv_obj_center(box);
    lv_obj_set_style_bg_color(box, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_border_width(box, 0, 0);
    lv_obj_set_style_radius(box, 0, 0);
    lv_obj_t *lbl = lv_label_create(scr);
    lv_label_set_text(lbl, "SparkDice 40");
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_28, 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 60);
    shot_render();

    assert(shot_pixel(180, 180) == 0xFF0000); /* box at center */
    assert(shot_pixel(180, 300) == 0x123456); /* background */
    assert(shot_save(out));

    /* PNG header: signature + IHDR 360x360 (LVGL's lodepng decoder returns a draw buf, not raw pixels) */
    unsigned char png[24];
    FILE *f = fopen(out, "rb");
    assert(f && fread(png, 1, sizeof(png), f) == sizeof(png));
    fclose(f);
    assert(!memcmp(png, "\x89PNG\r\n\x1a\n", 8) && !memcmp(png + 12, "IHDR", 4));
    assert(png[16] == 0 && png[17] == 0 && png[18] == 0x01 && png[19] == 0x68); /* width 360 */
    assert(png[20] == 0 && png[21] == 0 && png[22] == 0x01 && png[23] == 0x68); /* height 360 */
    printf("shot: wrote %s\n", out);
    return 0;
}
