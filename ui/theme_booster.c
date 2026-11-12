#include "theme.h"

/* Barlow Condensed (OFL, ui/assets/OFL.txt), embedded as binary objects by the build */
extern const uint8_t _binary_BarlowCondensed_ExtraBold_ttf_start[], _binary_BarlowCondensed_ExtraBold_ttf_end[];
extern const uint8_t _binary_BarlowCondensed_SemiBold_ttf_start[], _binary_BarlowCondensed_SemiBold_ttf_end[];


const ui_theme_t ui_theme_booster = {
    .ground = 0x121014,
    .ink = 0xFFF6E8,
    .seat = {0xE4572E, 0x179C8E, 0xF2B134, 0x8A6CD0},
    .seat_ink = {0x121014, 0x121014, 0x121014, 0x121014},
    .foil_a = 0xB9BEC6,
    .foil_b = 0xF3F4F6,
    .foil_ink = 0x121014,
    .rule_w = 2,
    .display_ttf = _binary_BarlowCondensed_ExtraBold_ttf_start,
    .display_ttf_end = _binary_BarlowCondensed_ExtraBold_ttf_end,
    .label_ttf = _binary_BarlowCondensed_SemiBold_ttf_start,
    .label_ttf_end = _binary_BarlowCondensed_SemiBold_ttf_end,
};
