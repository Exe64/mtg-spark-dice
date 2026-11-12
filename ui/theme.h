/* Visual tokens. One theme ships (booster wrapper); a theme selector is in the roadmap backlog.
 * ponytail: tokens only; per-theme signature drawing becomes a function pointer when a 2nd theme lands */
#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t ground;      /* wrapper interior */
    uint32_t ink;         /* text on fields and ground */
    uint32_t seat[4];     /* seat fields J1..J4 */
    uint32_t seat_ink[4]; /* text on each seat field (contrast >= 4.5:1) */
    uint32_t foil_a, foil_b, foil_ink; /* pending band only */
    int rule_w;           /* every rule, crimp and divider */
    const uint8_t *display_ttf, *display_ttf_end; /* numerals */
    const uint8_t *label_ttf, *label_ttf_end;     /* uppercase labels */
} ui_theme_t;

extern const ui_theme_t ui_theme_booster;
