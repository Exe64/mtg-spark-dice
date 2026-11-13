#include "theme.h"

extern const uint8_t _binary_Sucker_Brush_ttf_start[], _binary_Sucker_Brush_ttf_end[];
extern const uint8_t _binary_BarlowCondensed_SemiBold_ttf_start[], _binary_BarlowCondensed_SemiBold_ttf_end[];

/* Direction contract: .impeccable/surfaces/ui-ui-c.md ("well of light").
 * seat_ink and the foil tokens only serve the screens still drawn as painted fields (menus: plan #5 step 5). */
const ui_theme_t ui_theme_lumineux = {
    .light = true,
    .ground = 0x18214A,
    .ground_rim = 0x060914,
    .ground_pick = true,
    .ink = 0xF4F7FF,
    .seat = {0xFF7A45, 0x3FD8C0, 0xFFC94D, 0xA98BFF},
    .seat_ink = {0x060914, 0x060914, 0x060914, 0x060914},
    .foil_a = 0xAEB9DC,
    .foil_b = 0xF4F7FF,
    .foil_ink = 0x060914,
    .rule_w = 2,
    .display_ttf = _binary_Sucker_Brush_ttf_start,
    .display_ttf_end = _binary_Sucker_Brush_ttf_end,
    .label_ttf = _binary_BarlowCondensed_SemiBold_ttf_start,
    .label_ttf_end = _binary_BarlowCondensed_SemiBold_ttf_end,
};

const uint32_t ui_grounds[UI_GROUND_COUNT] = {
    0x18214A, /* Nuit */
    0x14151A, /* Noir */
    0x2B1A4A, /* Améthyste */
    0x12382B, /* Forêt */
    0x4A1826, /* Grenat */
    0x28323F, /* Ardoise */
};

/* 27 % of each channel: maps the contract's centre #18214A to its rim #060914 */
uint32_t ui_ground_rim(uint32_t hue)
{
    uint32_t rim = 0;
    for (int s = 16; s >= 0; s -= 8)
        rim |= ((((hue >> s) & 0xFF) * 27 + 50) / 100) << s;
    return rim;
}
