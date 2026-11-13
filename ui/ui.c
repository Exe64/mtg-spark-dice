#include "ui.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fx.h"
#include "lvgl.h"
#include "strings_fr.h"
#include "theme.h"

#define CX 180
#define CY 180
#define R 180
#define HUB_R 48
#define MINUS "\xE2\x88\x92" /* U+2212 */
#define MENU_ROWS 5
#define LOG_ROWS 5
#define UNDO_PILL_MS 4000
#define PM_TAP 26 /* |local x| beyond which a tap on the selected seat is - or + */

enum { NEW_FORMAT, NEW_PLAYERS }; /* the two steps of UI_SCR_NEW */
enum { MENU_UNDO, MENU_LOG, MENU_SHEET, MENU_NEW, MENU_BACK };

static const ui_theme_t *const THEMES[UI_THEME_COUNT] = {[UI_THEME_BOOSTER] = &ui_theme_booster,
                                                         [UI_THEME_LUMINEUX] = &ui_theme_lumineux};
static ui_theme_t TH; /* active theme, with the user's ground applied */
static const ui_theme_t *T = &ui_theme_lumineux; /* == *THEMES[UI_THEME_DEFAULT] until ui_set_theme() */

static struct {
    ui_hooks_t hooks;
    const sd_profile_t *profiles;
    int n_profiles;
    int profile, players, new_step;
    int marker; /* focused chip of the markers panel: kind - SD_POISON */
    bool has_resume;
    sd_game_t resume;
    sd_game_t g;
    char profile_name[24];
    uint8_t sel;
    ui_screen_t scr;
    int menu_focus, log_scroll;
    uint32_t now, undo_until, boot_until;
    lv_obj_t *dial;
    lv_font_t *f_solo, *f_arena[SD_MAX_PLAYERS + 1], *f_hub, *f_title, *f_name, *f_row, *f_label, *f_small;
} U;

/* ---------- drawing helpers ---------- */

static lv_color_t C(uint32_t hex) { return lv_color_hex(hex); }

static void fill(lv_layer_t *l, int x1, int y1, int x2, int y2, uint32_t color, int radius)
{
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.bg_color = C(color);
    d.radius = radius;
    lv_area_t a = {x1, y1, x2, y2};
    lv_draw_rect(l, &d, &a);
}

/* The radial well (centre -> rim) is computed once per ground colour into a screen-sized buffer in the display's
 * own pixel format, then copied: a per-frame radial gradient cost 8x a flat fill (host/bench.c). On 16-bit displays
 * it is dithered, because 5-6 bit channels would show rings in so dark a gradient. */
static lv_draw_buf_t *well_buf;
static uint32_t well_c, well_rim;

static const lv_draw_buf_t *well(void)
{
    if (well_buf && well_c == T->ground && well_rim == T->ground_rim)
        return well_buf;
    if (!well_buf)
        well_buf = lv_draw_buf_create(UI_W, UI_H, LV_COLOR_FORMAT_NATIVE, 0);
    if (!well_buf)
        return NULL;
    static const uint8_t bayer[4][4] = {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};
    (void)bayer;
    for (int y = 0; y < UI_H; y++) {
        uint8_t *rowp = well_buf->data + (size_t)y * well_buf->header.stride;
        for (int x = 0; x < UI_W; x++) {
            float d = sqrtf((float)((x - CX) * (x - CX) + (y - CY) * (y - CY))) / R;
            int k = d >= 1.f ? 256 : (int)(d * 256.f), ch[3]; /* ch = R, G, B in 0..255 */
            for (int c = 0; c < 3; c++) {
                int a = (int)((T->ground >> (16 - 8 * c)) & 0xFF), b = (int)((T->ground_rim >> (16 - 8 * c)) & 0xFF);
                ch[c] = a + (b - a) * k / 256;
            }
#if LV_COLOR_DEPTH == 16
            int t = bayer[y & 3][x & 3]; /* ordered dither: 0..15 sixteenths of one output step */
            int r5 = (ch[0] * 31 * 16 / 255 + t) >> 4, g6 = (ch[1] * 63 * 16 / 255 + t) >> 4,
                b5 = (ch[2] * 31 * 16 / 255 + t) >> 4;
            ((uint16_t *)rowp)[x] = (uint16_t)((r5 << 11) | (g6 << 5) | b5);
#else
            rowp[4 * x] = (uint8_t)ch[2];
            rowp[4 * x + 1] = (uint8_t)ch[1];
            rowp[4 * x + 2] = (uint8_t)ch[0];
            rowp[4 * x + 3] = 0xFF;
#endif
        }
    }
    well_c = T->ground;
    well_rim = T->ground_rim;
    return well_buf;
}

/* Full-dial ground: flat, or the theme's radial well. */
static void ground(lv_layer_t *l)
{
    lv_area_t a = {0, 0, UI_W - 1, UI_H - 1};
    const lv_draw_buf_t *w = T->ground_rim != T->ground ? well() : NULL;
    if (w) {
        lv_draw_image_dsc_t d;
        lv_draw_image_dsc_init(&d);
        d.src = w;
        lv_draw_image(l, &d, &a);
        return;
    }
    lv_draw_rect_dsc_t d; /* flat theme, or no memory for the well: the centre colour */
    lv_draw_rect_dsc_init(&d);
    d.bg_color = C(T->ground);
    lv_draw_rect(l, &d, &a);
}

static void outline(lv_layer_t *l, int x1, int y1, int x2, int y2, uint32_t color)
{
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.bg_opa = LV_OPA_TRANSP;
    d.border_color = C(color);
    d.border_width = T->rule_w;
    lv_area_t a = {x1, y1, x2, y2};
    lv_draw_rect(l, &d, &a);
}

static void foil(lv_layer_t *l, int x1, int y1, int x2, int y2, int radius)
{
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.radius = radius;
    d.bg_grad.dir = LV_GRAD_DIR_HOR;
    d.bg_grad.stops_count = 3;
    d.bg_grad.stops[0] = (lv_gradient_stop_t){.color = C(T->foil_a), .opa = LV_OPA_COVER, .frac = 0};
    d.bg_grad.stops[1] = (lv_gradient_stop_t){.color = C(T->foil_b), .opa = LV_OPA_COVER, .frac = 150};
    d.bg_grad.stops[2] = (lv_gradient_stop_t){.color = C(T->foil_a), .opa = LV_OPA_COVER, .frac = 255};
    lv_area_t a = {x1, y1, x2, y2};
    lv_draw_rect(l, &d, &a);
}

static void text(lv_layer_t *l, const lv_font_t *f, uint32_t color, lv_opa_t opa, int cx, int cy, const char *s)
{
    lv_draw_label_dsc_t d;
    lv_draw_label_dsc_init(&d);
    d.font = f;
    d.color = C(color);
    d.opa = opa;
    d.align = LV_TEXT_ALIGN_CENTER;
    d.text = s;
    d.text_local = 1;
    int h = lv_font_get_line_height(f);
    lv_area_t a = {cx - R, cy - h / 2, cx + R, cy + h / 2};
    lv_draw_label(l, &d, &a);
}

/* Largest display size whose width fits the well at the title line (the brush face runs wide) */
static const lv_font_t *title_font(const char *s)
{
    const lv_font_t *f[] = {U.f_title, U.f_hub, U.f_name};
    for (int i = 0; i < 2; i++)
        if (lv_text_get_width(s, strlen(s), f[i], 0) <= 300)
            return f[i];
    return f[2];
}

static void line(lv_layer_t *l, float x0, float y0, float x1, float y1, uint32_t color, lv_opa_t opa)
{
    lv_draw_line_dsc_t d;
    lv_draw_line_dsc_init(&d);
    d.color = C(color);
    d.opa = opa;
    d.width = T->rule_w;
    d.p1.x = x0;
    d.p1.y = y0;
    d.p2.x = x1;
    d.p2.y = y1;
    lv_draw_line(l, &d);
}

static void tri(lv_layer_t *l, float ax, float ay, float bx, float by, float cx, float cy, uint32_t color)
{
    lv_draw_triangle_dsc_t d;
    lv_draw_triangle_dsc_init(&d);
    d.bg_color = C(color);
    d.p[0].x = ax;
    d.p[0].y = ay;
    d.p[1].x = bx;
    d.p[1].y = by;
    d.p[2].x = cx;
    d.p[2].y = cy;
    lv_draw_triangle(l, &d);
}

static void ring(lv_layer_t *l, int r, float a0, float a1, uint32_t color)
{
    if (a1 <= a0)
        return;
    if (a0 < 0) { /* LVGL 9.2 does not normalise negative angles */
        a0 += 360.f;
        a1 += 360.f;
    }
    lv_draw_arc_dsc_t d;
    lv_draw_arc_dsc_init(&d);
    d.color = C(color);
    d.width = T->rule_w;
    d.center.x = CX;
    d.center.y = CY;
    d.radius = r;
    d.start_angle = a0;
    d.end_angle = a1;
    lv_draw_arc(l, &d);
}

static void wedge(lv_layer_t *l, float a0, float a1, uint32_t color)
{
    lv_draw_arc_dsc_t d;
    lv_draw_arc_dsc_init(&d);
    d.color = C(color);
    d.width = R;
    d.center.x = CX;
    d.center.y = CY;
    d.radius = R;
    d.start_angle = a0;
    d.end_angle = a1;
    lv_draw_arc(l, &d);
}

/* ---------- formatting ---------- */

static void fmt_signed(char *buf, size_t n, int v)
{
    if (v < 0)
        snprintf(buf, n, MINUS "%d", -v);
    else
        snprintf(buf, n, "+%d", v);
}

#define N_MARKERS (SD_KIND_COUNT - SD_POISON)
static const char *const MARKER_NAME[N_MARKERS] = S_MARKER_NAMES;
static const char *const MARKER_ABBR[N_MARKERS] = S_MARKER_ABBR;

/* who a state names: "J3", or a dash for nobody */
static const char *holder(char *buf, size_t n, uint8_t who)
{
    if (who >= U.g.n_players)
        return S_NONE;
    snprintf(buf, n, S_SEAT_FMT, who + 1);
    return buf;
}

static void fmt_action(char *buf, size_t n, const sd_action_t *a)
{
    char d[12];
    fmt_signed(d, sizeof(d), a->amount);
    if (a->kind == SD_TURN)
        snprintf(buf, n, S_TURN_FMT "  J%d" S_SEP "J%d", a->turn, a->player + 1, a->src + 1);
    else if (a->kind == SD_LIFE && a->src != a->player)
        snprintf(buf, n, "J%d" S_SEP "J%d  PV %s", a->src + 1, a->player + 1, d);
    else if (a->kind == SD_LIFE)
        snprintf(buf, n, "J%d  PV %s", a->player + 1, d);
    else if (a->kind == SD_CMD || a->kind == SD_CMD2)
        snprintf(buf, n, "J%d  %s J%d %s", a->player + 1, a->kind == SD_CMD ? S_CMD : S_CMD2, a->src + 1, d);
    else if (a->kind < SD_ASCEND)
        snprintf(buf, n, "J%d  %s %s", a->player + 1, MARKER_NAME[a->kind - SD_POISON], d);
    else if (a->kind == SD_ASCEND)
        snprintf(buf, n, "J%d  %s %s", a->player + 1, MARKER_NAME[a->kind - SD_POISON], a->amount > 0 ? "+" : MINUS);
    else if (a->kind == SD_DAYNIGHT)
        snprintf(buf, n, "%s", a->player == SD_DN_DAY ? S_DAY : a->player == SD_DN_NIGHT ? S_NIGHT : S_DAY " / " S_NIGHT "  " S_NONE);
    else
        snprintf(buf, n, "%s  %s", MARKER_NAME[a->kind - SD_POISON], holder(d, sizeof(d), a->player));
}

/* What the undo pill takes back, in a few characters. */
static void fmt_undo(char *buf, size_t n, const sd_action_t *a)
{
    if (a->kind == SD_TURN) /* the hand-off it reverses */
        snprintf(buf, n, "J%d" S_SEP "J%d", a->player + 1, a->src + 1);
    else if (a->kind >= SD_ASCEND)
        snprintf(buf, n, "%s", MARKER_ABBR[a->kind - SD_POISON]);
    else
        fmt_signed(buf, n, a->kind == SD_CMD || a->kind == SD_CMD2 ? -a->amount : a->amount);
}

/* The seat a journal line belongs to, as a colour; ink when it belongs to the table (day/night, nobody). */
static uint32_t log_hue(const sd_action_t *a)
{
    uint8_t who = a->kind == SD_DAYNIGHT ? SD_NOBODY : a->kind < SD_MONARCH || a->player != SD_NOBODY ? a->player : a->src;
    return who < U.g.n_players ? T->seat[who] : T->ink;
}

static float pending_progress(void)
{
    uint32_t el = U.now - U.g.pending_since;
    return el >= SD_COMMIT_MS ? 1.f : (float)el / SD_COMMIT_MS;
}

/* ---------- geometry ---------- */

static float seat_span(void) { return 360.f / U.g.n_players; }
static float seat_angle(int i) { return 90.f + i * seat_span(); } /* J1 sits at the bottom, then clockwise */

static int seat_at(int x, int y)
{
    if (U.g.n_players == 1)
        return 0;
    float dx = x - CX, dy = y - CY;
    if (dx * dx + dy * dy < HUB_R * HUB_R || dx * dx + dy * dy > R * R)
        return -1;
    float a = atan2f(dy, dx) * 180.f / (float)M_PI;
    float rel = fmodf(a - 90.f + seat_span() / 2 + 720.f, 360.f);
    return (int)(rel / seat_span()) % U.g.n_players;
}

static bool in_hub(int x, int y) { return (x - CX) * (x - CX) + (y - CY) * (y - CY) <= HUB_R * HUB_R; }

/* ---------- game actions ---------- */

static void haptic(ui_haptic_t p)
{
    if (U.hooks.haptic)
        U.hooks.haptic(p);
}

static void save(void)
{
    if (U.hooks.save)
        U.hooks.save(&U.g);
}

static void redraw(void) { lv_obj_invalidate(U.dial); }

/* Arena seats show committed values: while only the pending delta / pill changes, repaint the hub, not the turned
 * seat layers (each is an ARGB8888 layer, costly on the S3). */
static void redraw_hub(void)
{
    if (U.scr != UI_SCR_GAME || U.g.n_players == 1) {
        redraw();
        return;
    }
    const int m = T->light ? 18 : 6; /* the luminous hub glows past its edge */
    lv_area_t a = {CX - HUB_R - m, CY - HUB_R - m, CX + HUB_R + m, CY + HUB_R + m};
    lv_obj_invalidate_area(U.dial, &a);
}

/* Called after any commit: persist, haptics, defeat and game-over detection. */
static void committed(bool was_lost[SD_MAX_PLAYERS])
{
    save();
    U.undo_until = U.now + UNDO_PILL_MS;
    bool new_loss = false;
    for (int i = 0; i < U.g.n_players; i++)
        new_loss |= !was_lost[i] && sd_player_lost(&U.g, (uint8_t)i);
    haptic(new_loss ? UI_HAPTIC_DEFEAT : UI_HAPTIC_COMMIT);
    if (sd_game_over(&U.g))
        U.scr = UI_SCR_OVER;

}

static void snapshot_lost(bool out[SD_MAX_PLAYERS])
{
    for (int i = 0; i < SD_MAX_PLAYERS; i++)
        out[i] = i < U.g.n_players && sd_player_lost(&U.g, (uint8_t)i);
}

static void flush(void)
{
    bool lost[SD_MAX_PLAYERS];
    snapshot_lost(lost);
    if (sd_game_flush(&U.g))
        committed(lost);
}

static void adjust(uint8_t player, sd_kind_t kind, uint8_t src, int step)
{
    bool lost[SD_MAX_PLAYERS];
    snapshot_lost(lost);
    if (sd_game_adjust(&U.g, player, kind, src, (int16_t)step, U.now))
        committed(lost);
    haptic(UI_HAPTIC_DETENT);
}

static bool undo_pill(void) { return !U.g.has_pending && U.g.log_len && U.now < U.undo_until; }

static void undo(void)
{
    if (sd_game_undo(&U.g)) {
        save();
        haptic(UI_HAPTIC_COMMIT);
    }
    U.undo_until = 0;
}

static void next_turn(void)
{
    bool lost[SD_MAX_PLAYERS];
    snapshot_lost(lost);
    if (sd_game_flush(&U.g))
        committed(lost);
    if (U.scr == UI_SCR_GAME && sd_game_next_turn(&U.g)) {
        save();
        U.undo_until = U.now + UNDO_PILL_MS;
        haptic(UI_HAPTIC_COMMIT);
    }
}

/* Selected seat's local frame: seats face their owner (rotation = seat angle - 90°). */
static float seat_rot(int i) { return U.g.n_players == 1 ? 0.f : seat_angle(i) - 90.f; }
static const float R_NUM[] = {0, 0, 96, 104, 108};
static float seat_local_x(int i, int x, int y)
{
    float a = seat_angle(i) * (float)M_PI / 180.f, r = seat_rot(i) * (float)M_PI / 180.f;
    float nx = CX + R_NUM[U.g.n_players] * cosf(a), ny = CY + R_NUM[U.g.n_players] * sinf(a);
    return (x - nx) * cosf(r) + (y - ny) * sinf(r);
}

static void upper(char *s)
{
    for (; *s; s++)
        if (*s >= 'a' && *s <= 'z')
            *s = (char)(*s - 32);
}

static void start_game(const sd_game_t *resume)
{
    if (resume) {
        U.g = *resume;
        snprintf(U.profile_name, sizeof(U.profile_name), "%s", U.g.format);
    } else {
        const sd_profile_t *p = &U.profiles[U.profile];
        sd_game_new(&U.g, &p->rules, (uint8_t)U.players);
        snprintf(U.g.format, sizeof(U.g.format), "%s", p->name);
        snprintf(U.profile_name, sizeof(U.profile_name), "%s", p->name);
        save();
    }
    upper(U.profile_name);
    U.sel = 0;
    U.has_resume = false;
    U.scr = sd_game_over(&U.g) ? UI_SCR_OVER : UI_SCR_GAME;
}

static void go_home(void)
{
    flush();
    U.resume = U.g; /* "Nouvelle partie" never destroys the game: it stays resumable */
    U.has_resume = true;
    U.scr = UI_SCR_HOME;
}

/* ---------- screens: drawing ---------- */

/* Direct way into the markers panel: a three-dot button (not a "+", which already means +1 life), on the right in
 * Solo and in the right corner of the selected seat's rim in the arena (right as its player sees it). */
#define MARKERS_BTN_R 17
static void markers_btn_xy(int *x, int *y)
{
    if (U.g.n_players == 1) {
        *x = 296;
        *y = 262;
        return;
    }
    float a = (seat_angle(U.sel) - seat_span() / 2 + 14.f) * (float)M_PI / 180.f;
    *x = CX + (int)((R - 28) * cosf(a));
    *y = CY + (int)((R - 28) * sinf(a));
}

static void markers_btn(lv_layer_t *l); /* drawn with button(), defined with the panel */

static bool markers_btn_shown(void) { return !sd_player_lost(&U.g, U.sel); }

static bool markers_btn_hit(int x, int y)
{
    int bx, by;
    markers_btn_xy(&bx, &by);
    return markers_btn_shown() && (x - bx) * (x - bx) + (y - by) * (y - by) <= 26 * 26; /* a finger, not the ink */
}

/* Pending band: dull silver that the foil sheen fills from left to right over SD_COMMIT_MS (the seal).
 * The bright leading edge is the timer; the edges are always complete. */
static void band(lv_layer_t *l, int y1, int y2, const char *txt)
{
    fill(l, 0, y1, UI_W - 1, y2, T->foil_a, 0);
    int xs = (int)(UI_W * pending_progress());
    if (xs > 1) {
        lv_draw_rect_dsc_t d;
        lv_draw_rect_dsc_init(&d);
        d.bg_grad.dir = LV_GRAD_DIR_HOR;
        d.bg_grad.stops_count = 2;
        d.bg_grad.stops[0] = (lv_gradient_stop_t){.color = C(T->foil_a), .opa = LV_OPA_COVER, .frac = 0};
        d.bg_grad.stops[1] = (lv_gradient_stop_t){.color = C(T->foil_b), .opa = LV_OPA_COVER, .frac = 255};
        lv_area_t a = {0, y1, xs - 1, y2};
        lv_draw_rect(l, &d, &a);
    }
    line(l, 0, y1, UI_W, y1, T->foil_ink, LV_OPA_COVER);
    line(l, 0, y2, UI_W, y2, T->foil_ink, LV_OPA_COVER);
    text(l, U.f_row, T->foil_ink, LV_OPA_COVER, CX, (y1 + y2) / 2, txt);
}

/* Defeated seat: the skull mark (ui/assets/dead.svg -> A8 mask, ui/skull_a8.c) tinted with the seat hue replaces
 * the total; `s` is its drawn size. */
extern const lv_image_dsc_t ui_skull_a8;
static void skull(lv_layer_t *l, int cx, int cy, int s, uint32_t color, lv_opa_t opa)
{
    const int n = (int)ui_skull_a8.header.w;
    lv_draw_image_dsc_t d;
    lv_draw_image_dsc_init(&d);
    d.src = &ui_skull_a8;
    d.recolor = C(color);
    d.recolor_opa = LV_OPA_COVER;
    d.opa = opa;
    d.scale_x = d.scale_y = (int32_t)(256 * s / n);
    d.pivot.x = n / 2;
    d.pivot.y = n / 2;
    lv_area_t a = {cx - n / 2, cy - n / 2, cx - n / 2 + n - 1, cy - n / 2 + n - 1};
    lv_draw_image(l, &d, &a);
}

/* drawn + and - marks (no glyphs), one rule weight */
static void pm_marks(lv_layer_t *l, int cx, int cy, int dx, uint32_t color)
{
    const int h = 9;
    line(l, cx - dx - h, cy, cx - dx + h, cy, color, LV_OPA_COVER);
    line(l, cx + dx - h, cy, cx + dx + h, cy, color, LV_OPA_COVER);
    line(l, cx + dx, cy - h, cx + dx, cy + h, color, LV_OPA_COVER);
}

static void draw_hub(lv_layer_t *l)
{
    const sd_game_t *g = &U.g;
    if (!g->has_pending) {
        char t[16];
        if (undo_pill()) { /* quick undo of the last commit, tap the hub */
            const sd_action_t *a = &g->log[g->log_len - 1];
            if (a->automatic && g->log_len > 1) /* an automatic pass is undone with the hit that caused it */
                a--;
            fill(l, CX - HUB_R, CY - HUB_R, CX + HUB_R, CY + HUB_R, T->ink, LV_RADIUS_CIRCLE);
            text(l, U.f_small, T->ground, LV_OPA_COVER, CX, CY - 14, S_MENU_UNDO);
            fmt_undo(t, sizeof(t), a);
            text(l, U.f_row, T->ground, LV_OPA_COVER, CX, CY + 12, t);
            return;
        }
        /* at rest the hub is sealed (everything committed) and shows whose turn it is; tap = next turn */
        fill(l, CX - HUB_R, CY - HUB_R, CX + HUB_R, CY + HUB_R, T->ground, LV_RADIUS_CIRCLE);
        ring(l, HUB_R, 0.f, 360.f, T->ink);
        snprintf(t, sizeof(t), S_TURN_FMT, g->turn);
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, CY - 20, t);
        snprintf(t, sizeof(t), S_SEAT_FMT, g->active + 1);
        text(l, U.f_hub, T->seat[g->active], LV_OPA_COVER, CX, CY + 8, t);
        return;
    }
    char d[12], sub[24];
    fmt_signed(d, sizeof(d), g->pending.kind == SD_CMD ? -g->pending.amount : g->pending.amount); /* effect on life */
    foil(l, CX - HUB_R, CY - HUB_R, CX + HUB_R, CY + HUB_R, LV_RADIUS_CIRCLE);
    text(l, U.f_hub, T->foil_ink, LV_OPA_COVER, CX, CY - 8, d);
    if (g->pending.kind == SD_CMD)
        text(l, U.f_small, T->foil_ink, LV_OPA_COVER, CX, CY - 36, S_CMD);
    if (g->pending.src != g->pending.player) { /* source seat, drawn pointer, target seat */
        snprintf(sub, sizeof(sub), S_SEAT_FMT, g->pending.src + 1);
        text(l, U.f_small, T->foil_ink, LV_OPA_COVER, CX - 17, CY + 26, sub);
        snprintf(sub, sizeof(sub), S_SEAT_FMT, g->pending.player + 1);
        text(l, U.f_small, T->foil_ink, LV_OPA_COVER, CX + 17, CY + 26, sub);
        tri(l, CX - 4, CY + 21, CX - 4, CY + 31, CX + 4, CY + 26, T->foil_ink);
    } else {
        snprintf(sub, sizeof(sub), "= %d", sd_game_view(g, g->pending.player).life);
        text(l, U.f_small, T->foil_ink, LV_OPA_COVER, CX, CY + 26, sub);
    }
    /* the seal: the ring closes clockwise over exactly SD_COMMIT_MS */
    ring(l, HUB_R, -90.f, -90.f + 360.f * pending_progress(), T->ink);
}

static int view_poison(const sd_game_t *g)
{
    sd_player_t v = sd_game_view(g, U.sel);
    return sd_counter(&v, SD_POISON);
}

/* "P3  C12" style counters; empty when none */
static void fmt_counters(char *buf, size_t n, const sd_player_t *p)
{
    int cmd = 0;
    for (int s = 0; s < U.g.n_players; s++)
        cmd = p->cmd[s][0] > cmd ? p->cmd[s][0] : cmd;
    buf[0] = '\0';
    if (sd_counter(p, SD_POISON) && cmd)
        snprintf(buf, n, "P%d  C%d", sd_counter(p, SD_POISON), cmd);
    else if (sd_counter(p, SD_POISON))
        snprintf(buf, n, S_POISON " %d", sd_counter(p, SD_POISON));
    else if (cmd)
        snprintf(buf, n, S_CMD " %d", cmd);
}

static void draw_solo(lv_layer_t *l)
{
    const sd_game_t *g = &U.g;
    const sd_player_t *p = &g->p[0];
    bool lost = sd_player_lost(g, 0);
    uint32_t field = lost ? T->ground : T->seat[0], ink = lost ? T->seat[0] : T->seat_ink[0];
    char buf[64];
    if (lost)
        ground(l);
    else
        fill(l, 0, 0, UI_W - 1, UI_H - 1, field, 0);
    snprintf(buf, sizeof(buf), S_SEAT_PROFILE_FMT, 1, U.profile_name);
    text(l, U.f_label, ink, LV_OPA_COVER, CX, 62, buf);
    snprintf(buf, sizeof(buf), "%d", p->life);
    if (lost)
        skull(l, CX, 158, 150, ink, LV_OPA_COVER);
    else
        text(l, U.f_solo, ink, LV_OPA_COVER, CX, 158, buf);
    if (!lost)
        pm_marks(l, CX, 162, 136, ink);
    char cnt[24];
    markers_btn(l);
    fmt_counters(cnt, sizeof(cnt), p);
    text(l, U.f_label, ink, LV_OPA_COVER, CX, 334, cnt);
    if (undo_pill()) {
        const sd_action_t *a = &g->log[g->log_len - 1];
        char d[12];
        fmt_undo(d, sizeof(d), a);
        snprintf(buf, sizeof(buf), S_MENU_UNDO "  %s", d);
        fill(l, 100, 282, 260, 318, T->ink, 0);
        text(l, U.f_label, T->ground, LV_OPA_COVER, CX, 300, buf);
    }
    if (!g->has_pending)
        return;
    /* pending band; the foil sheen fills it over the 800 ms */
    const int y1 = 250, y2 = 296;
    char d[12];
    fmt_signed(d, sizeof(d), g->pending.amount);
    snprintf(buf, sizeof(buf), "%s  = %d", d, sd_game_view(g, 0).life);
    band(l, y1, y2, buf);
}

static void draw_arena(lv_layer_t *l)
{
    const sd_game_t *g = &U.g;
    int n = g->n_players;
    float span = seat_span();
    ground(l);
    if (!sd_player_lost(g, U.sel))
        wedge(l, seat_angle(U.sel) - span / 2, seat_angle(U.sel) + span / 2, T->seat[U.sel]);
    for (int i = 0; i < n; i++) { /* straight seams between seats */
        float a = (seat_angle(i) - span / 2) * (float)M_PI / 180.f;
        line(l, CX + HUB_R * cosf(a), CY + HUB_R * sinf(a), CX + R * cosf(a), CY + R * sinf(a), T->ink, LV_OPA_COVER);
    }
    /* active seat: a solid band along its rim */
    float aa = seat_angle(g->active);
    lv_draw_arc_dsc_t crest;
    lv_draw_arc_dsc_init(&crest);
    crest.color = C((g->active == U.sel && !sd_player_lost(g, g->active)) ? T->seat_ink[g->active] : T->ink);
    crest.width = 6;
    crest.center.x = CX;
    crest.center.y = CY;
    crest.radius = R - 4;
    crest.start_angle = aa - span / 2 + 6;
    crest.end_angle = aa + span / 2 - 6;
    lv_draw_arc(l, &crest);
    for (int i = 0; i < n; i++) {
        bool sel = i == U.sel, lost = sd_player_lost(g, (uint8_t)i);
        uint32_t ink = (sel && !lost) ? T->seat_ink[i] : T->seat[i];
        float a = seat_angle(i) * (float)M_PI / 180.f, rot = seat_rot(i);
        int nx = CX + (int)(R_NUM[n] * cosf(a)), ny = CY + (int)(R_NUM[n] * sinf(a));
        int lh = lv_font_get_line_height(U.f_arena[n]);
        /* each seat is drawn upright in its own layer, then turned to face its player */
        const int bw = n == 2 ? 300 : 190, bh = lh + 44;
        lv_area_t box = {nx - bw / 2, ny - bh / 2, nx - bw / 2 + bw - 1, ny - bh / 2 + bh - 1};
        bool turn = fabsf(rot) > 0.5f;
        lv_layer_t *ly = turn ? lv_draw_layer_create(l, LV_COLOR_FORMAT_ARGB8888, &box) : l;
        char buf[40], cnt[24];
        snprintf(buf, sizeof(buf), "%d", g->p[i].life);
        if (lost) /* defeat: a skull replaces the total, in full hue (the total stays in the log, undo restores it) */
            skull(ly, nx, ny - 14, lh * 84 / 100, ink, LV_OPA_COVER);
        else
            text(ly, U.f_arena[n], ink, LV_OPA_COVER, nx, ny - 8, buf);
        if (sel && !lost)
            pm_marks(ly, nx, ny - 6, n == 2 ? 110 : n == 3 ? 74 : 64, ink);
        fmt_counters(cnt, sizeof(cnt), &g->p[i]);
        if (lost)
            snprintf(buf, sizeof(buf), S_SEAT_FMT "  " S_DEFEATED, i + 1);
        else if (cnt[0])
            snprintf(buf, sizeof(buf), S_SEAT_FMT "  %s", i + 1, cnt);
        else
            snprintf(buf, sizeof(buf), S_SEAT_FMT, i + 1);
        text(ly, U.f_label, ink, LV_OPA_COVER, nx, ny + lh / 2 - 8, buf); /* rim side, inside the crest */
        if (turn) {
            lv_draw_image_dsc_t d;
            lv_draw_image_dsc_init(&d);
            d.src = ly;
            d.rotation = (int32_t)lroundf(rot * 10.f);
            d.pivot.x = bw / 2;
            d.pivot.y = bh / 2;
            lv_draw_layer(l, &d, &box);
        }
    }
    markers_btn(l);
    draw_hub(l);
}

/* ---------- luminous theme: game screens (contract: .impeccable/surfaces/ui-ui-c.md) ---------- */

static void arc(lv_layer_t *l, int r, float a0, float a1, int w, uint32_t color, lv_opa_t opa)
{
    if (a1 <= a0)
        return;
    if (a0 < 0) { /* LVGL 9.2 does not normalise negative angles */
        a0 += 360.f;
        a1 += 360.f;
    }
    lv_draw_arc_dsc_t d;
    lv_draw_arc_dsc_init(&d);
    d.color = C(color);
    d.opa = opa;
    d.width = w;
    d.center.x = CX;
    d.center.y = CY;
    d.radius = r;
    d.start_angle = a0;
    d.end_angle = a1;
    lv_draw_arc(l, &d);
}

/* Glass pill; `lit` = it is the thing to tap right now. */
static void pill(lv_layer_t *l, int x1, int y1, int x2, int y2, bool lit)
{
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.bg_color = C(T->ground_rim);
    d.bg_opa = 215;
    d.radius = LV_RADIUS_CIRCLE;
    d.border_color = C(T->ink);
    d.border_opa = lit ? LV_OPA_80 : LV_OPA_30;
    d.border_width = lit ? 2 : 1;
    lv_area_t a = {x1, y1, x2, y2};
    lv_draw_rect(l, &d, &a);
}

/* The knob's knurl: a thin circle near the rim with 16 detent ticks. */
#define KNOB_R (R - 10)
static void knob_ring(lv_layer_t *l)
{
    arc(l, KNOB_R, 0.f, 360.f, 1, T->ink, LV_OPA_40);
    for (int i = 0; i < 16; i++) {
        float a = i * 22.5f * (float)M_PI / 180.f;
        line(l, CX + KNOB_R * cosf(a), CY + KNOB_R * sinf(a), CX + (R - 3) * cosf(a), CY + (R - 3) * sinf(a), T->ink,
             LV_OPA_40);
    }
}

static void draw_hub_light(lv_layer_t *l)
{
    const sd_game_t *g = &U.g;
    char t[16];
    if (!g->has_pending && undo_pill()) { /* quick undo of the last commit: the hub lights up white */
        const sd_action_t *a = &g->log[g->log_len - 1];
        if (a->automatic && g->log_len > 1) /* an automatic pass is undone with the hit that caused it */
            a--;
        fx_glass(l, CX, CY, HUB_R, T->ground_rim, true);
        fx_icon(l, FX_ICON_UNDO, CX, CY - 17, T->ink, LV_OPA_COVER);
        fmt_undo(t, sizeof(t), a);
        text(l, U.f_row, T->ink, LV_OPA_COVER, CX, CY + 20, t);
        return;
    }
    /* the hub is the light source: a glass disc inside its own glow */
    fx_ring(l, CX, CY, HUB_R, T->ink, g->has_pending ? LV_OPA_30 : LV_OPA_70);
    fill(l, CX - HUB_R, CY - HUB_R, CX + HUB_R, CY + HUB_R, T->ground_rim, LV_RADIUS_CIRCLE);
    if (!g->has_pending) { /* at rest: whose turn it is; tap = next turn */
        snprintf(t, sizeof(t), S_TURN_FMT, g->turn);
        text(l, U.f_label, T->ink, LV_OPA_COVER, CX, CY - 22, t);
        snprintf(t, sizeof(t), S_SEAT_FMT, g->active + 1);
        text(l, U.f_hub, T->seat[g->active], LV_OPA_COVER, CX, CY + 9, t);
        return;
    }
    char d[12], sub[24];
    bool cmd = g->pending.kind == SD_CMD;
    fmt_signed(d, sizeof(d), cmd ? -g->pending.amount : g->pending.amount); /* effect on life */
    text(l, U.f_hub, T->ink, LV_OPA_COVER, CX, cmd ? CY - 3 : CY - 8, d);
    if (cmd) /* the only mark that tells commander damage from combat damage: kept clear of the ring */
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, CY - 31, S_CMD);
    if (g->pending.src != g->pending.player) { /* source seat, drawn pointer, target seat: each in its own light */
        snprintf(sub, sizeof(sub), S_SEAT_FMT, g->pending.src + 1);
        text(l, U.f_small, T->seat[g->pending.src], LV_OPA_COVER, CX - 17, CY + 26, sub);
        snprintf(sub, sizeof(sub), S_SEAT_FMT, g->pending.player + 1);
        text(l, U.f_small, T->seat[g->pending.player], LV_OPA_COVER, CX + 17, CY + 26, sub);
        tri(l, CX - 4, CY + 21, CX - 4, CY + 31, CX + 4, CY + 26, T->ink);
    } else {
        snprintf(sub, sizeof(sub), "= %d", sd_game_view(g, g->pending.player).life);
        text(l, U.f_label, T->ink, LV_OPA_COVER, CX, CY + 27, sub);
    }
    /* the light ring closes clockwise over exactly SD_COMMIT_MS, on the edge of the glass */
    arc(l, HUB_R + 3, -90.f, -90.f + 360.f * pending_progress(), 4, T->ink, LV_OPA_COVER);
}

static void draw_solo_light(lv_layer_t *l)
{
    const sd_game_t *g = &U.g;
    const sd_player_t *p = &g->p[0];
    bool lost = sd_player_lost(g, 0);
    lv_opa_t opa = lost ? LV_OPA_60 : LV_OPA_COVER; /* a defeated seat goes dark */
    char buf[64];
    ground(l);
    knob_ring(l);
    if (!lost)
        fx_halo(l, CX, 160, 150, T->seat[0], LV_OPA_40);
    snprintf(buf, sizeof(buf), S_SEAT_PROFILE_FMT, 1, U.profile_name);
    text(l, U.f_label, lost ? T->ink : T->seat[0], opa, CX, 62, buf);
    snprintf(buf, sizeof(buf), "%d", p->life);
    if (lost) {
        skull(l, CX, 158, 150, T->ink, opa);
        text(l, U.f_label, T->ink, opa, CX, 262, S_DEFEATED);
    } else {
        text(l, U.f_solo, T->ink, LV_OPA_COVER, CX, 158, buf);
        pm_marks(l, CX, 162, 136, T->ink);
    }
    char cnt[24];
    markers_btn(l);
    fmt_counters(cnt, sizeof(cnt), p);
    text(l, U.f_label, T->ink, opa, CX, 326, cnt);
    if (undo_pill()) {
        const sd_action_t *a = &g->log[g->log_len - 1];
        char d[12];
        fmt_undo(d, sizeof(d), a);
        snprintf(buf, sizeof(buf), S_MENU_UNDO "  %s", d);
        pill(l, 100, 282, 260, 318, true);
        text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 300, buf);
    }
    if (!g->has_pending)
        return;
    /* pending: the delta waits in a glass pill while a light arc runs once around the knob ring */
    char d[12];
    fmt_signed(d, sizeof(d), g->pending.amount);
    snprintf(buf, sizeof(buf), "%s  = %d", d, sd_game_view(g, 0).life);
    pill(l, 96, 252, 264, 296, false);
    text(l, U.f_row, T->ink, LV_OPA_COVER, CX, 274, buf);
    arc(l, KNOB_R + 2, -90.f, -90.f + 360.f * pending_progress(), 5, T->ink, LV_OPA_COVER);
}

static void draw_arena_light(lv_layer_t *l)
{
    const sd_game_t *g = &U.g;
    int n = g->n_players;
    float span = seat_span();
    const float gap = 2.f; /* whole degrees trimmed on each side (LVGL truncates arc angles): seats are parted by
                           * ground, not by rules */
    const int r_out = R - 2, r_in = HUB_R + 8;
    ground(l);
    for (int i = 0; i < n; i++) { /* glass sectors, tinted by the seat's light */
        if (sd_player_lost(g, (uint8_t)i))
            continue; /* a defeated seat is switched off */
        float a = seat_angle(i);
        /* selected glass is only a little brighter (46 vs 36 of 255): with its halo, more would take the white
         * numeral under 7:1 on the gold seat. The selection is carried by a full-hue band on the rim instead. */
        arc(l, r_out, a - span / 2 + gap, a + span / 2 - gap, r_out - r_in, T->seat[i], i == U.sel ? 46 : 36);
        if (i == U.sel)
            arc(l, r_out, a - span / 2 + gap, a + span / 2 - gap, 5, T->seat[i], LV_OPA_COVER);
    }
    if (!sd_player_lost(g, U.sel)) { /* the selected seat carries its halo */
        float a = seat_angle(U.sel) * (float)M_PI / 180.f;
        fx_halo(l, CX + (int)(R_NUM[n] * cosf(a)), CY + (int)(R_NUM[n] * sinf(a)), n == 2 ? 120 : 84, T->seat[U.sel],
                LV_OPA_20); /* kept low: white numerals need 7:1 over it */
    }
    { /* whose turn it is: a thin line of the seat's light along the three sides of its zone (the two edges and the
       * rim). The selected seat keeps the thick rim band, so the two marks read apart and stack on one seat. */
        float a0 = seat_angle(g->active) - span / 2 + gap, a1 = a0 + span - 2 * gap;
        arc(l, r_out, a0, a1, T->rule_w, T->seat[g->active], LV_OPA_COVER);
        for (int k = 0; k < 2; k++) {
            float a = (k ? a1 : a0) * (float)M_PI / 180.f;
            line(l, CX + r_in * cosf(a), CY + r_in * sinf(a), CX + r_out * cosf(a), CY + r_out * sinf(a),
                 T->seat[g->active], LV_OPA_COVER);
        }
    }
    for (int i = 0; i < n; i++) {
        bool sel = i == U.sel, lost = sd_player_lost(g, (uint8_t)i);
        lv_opa_t opa = lost ? LV_OPA_60 : LV_OPA_COVER;
        float a = seat_angle(i) * (float)M_PI / 180.f, rot = seat_rot(i);
        int nx = CX + (int)(R_NUM[n] * cosf(a)), ny = CY + (int)(R_NUM[n] * sinf(a));
        int lh = lv_font_get_line_height(U.f_arena[n]);
        /* each seat is drawn upright in its own layer, then turned to face its player */
        const int bw = n == 2 ? 300 : 190, bh = lh + 44;
        lv_area_t box = {nx - bw / 2, ny - bh / 2, nx - bw / 2 + bw - 1, ny - bh / 2 + bh - 1};
        bool turn = fabsf(rot) > 0.5f;
        lv_layer_t *ly = turn ? lv_draw_layer_create(l, LV_COLOR_FORMAT_ARGB8888, &box) : l;
        char buf[40], cnt[24];
        snprintf(buf, sizeof(buf), "%d", g->p[i].life);
        if (lost) /* the skull replaces the total, in dimmed ink (the total stays in the log, undo restores it) */
            skull(ly, nx, ny - 14, lh * 84 / 100, T->ink, opa);
        else
            text(ly, U.f_arena[n], T->ink, LV_OPA_COVER, nx, ny - 8, buf);
        if (sel && !lost)
            pm_marks(ly, nx, ny - 6, n == 2 ? 110 : n == 3 ? 74 : 64, T->ink);
        fmt_counters(cnt, sizeof(cnt), &g->p[i]);
        if (lost)
            snprintf(buf, sizeof(buf), S_SEAT_FMT "  " S_DEFEATED, i + 1);
        else if (cnt[0])
            snprintf(buf, sizeof(buf), S_SEAT_FMT "  %s", i + 1, cnt);
        else
            snprintf(buf, sizeof(buf), S_SEAT_FMT, i + 1);
        /* 30 px label (readable across the table); white on the lit glass of the selected seat */
        text(ly, U.f_row, lost || sel ? T->ink : T->seat[i], opa, nx, ny + lh / 2 - 6, buf);
        if (turn) {
            lv_draw_image_dsc_t d;
            lv_draw_image_dsc_init(&d);
            d.src = ly;
            d.rotation = (int32_t)lroundf(rot * 10.f);
            d.pivot.x = bw / 2;
            d.pivot.y = bh / 2;
            lv_draw_layer(l, &d, &box);
        }
    }
    markers_btn(l);
    draw_hub_light(l);
}

static int menu_row_y(int i) { return 76 + i * 52; }

/* Text of menu row `i` (the undo row previews what it would undo). */
static const char *menu_label(int i, char *buf, size_t n)
{
    char act[40];
    switch (i) {
    case MENU_UNDO:
        if (!U.g.log_len)
            return S_NOTHING_TO_UNDO;
        {
            const sd_action_t *last = &U.g.log[U.g.log_len - 1];
            if (last->automatic && U.g.log_len > 1) /* undone together with the hit that caused it */
                last--;
            fmt_action(act, sizeof(act), last);
            snprintf(buf, n, S_MENU_UNDO "  %s", act);
        }
        return buf;
    case MENU_LOG: return S_MENU_LOG;
    case MENU_SHEET:
        snprintf(buf, n, S_MENU_SHEET_FMT, U.sel + 1);
        return buf;
    case MENU_NEW: return S_MENU_NEW;
    default: return S_MENU_BACK;
    }
}

static void draw_menu(lv_layer_t *l)
{
    char buf[64];
    ground(l);
    for (int i = 0; i < MENU_ROWS; i++) {
        int y = menu_row_y(i);
        const char *s = menu_label(i, buf, sizeof(buf));
        bool f = i == U.menu_focus;
        if (f)
            fill(l, 36, y - 24, UI_W - 37, y + 23, T->ink, 0);
        text(l, U.f_row, f ? T->ground : T->ink, LV_OPA_COVER, CX, y, s);
    }
}

/* One left-aligned journal line at height y. */
static void log_row(lv_layer_t *l, const sd_action_t *a, int y)
{
    char buf[64], act[48];
    fmt_action(act, sizeof(act), a);
    if (a->kind == SD_TURN)
        snprintf(buf, sizeof(buf), "%s", act);
    else
        snprintf(buf, sizeof(buf), "T%d  %s", a->turn, act);
    lv_draw_label_dsc_t d;
    lv_draw_label_dsc_init(&d);
    d.font = U.f_label;
    d.color = C(T->ink);
    d.text = buf;
    d.text_local = 1;
    int h = lv_font_get_line_height(U.f_label);
    lv_area_t ar = {96, y - h / 2, 310, y + h / 2};
    lv_draw_label(l, &d, &ar);
}

static void draw_log(lv_layer_t *l)
{
    ground(l);
    text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 48, S_MENU_LOG);
    line(l, 120, 70, 240, 70, T->ink, LV_OPA_COVER);
    if (!U.g.log_len)
        text(l, U.f_row, T->ink, LV_OPA_COVER, CX, CY, S_LOG_EMPTY);
    for (int k = 0; k < LOG_ROWS; k++) {
        int idx = U.g.log_len - 1 - U.log_scroll - k;
        if (idx < 0)
            break;
        const sd_action_t *a = &U.g.log[idx];
        int y = 104 + k * 44;
        fill(l, 70, y - 7, 84, y + 7, log_hue(a), 0);
        log_row(l, a, y);
    }
    text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 326, S_TAP_BACK);
}

static void draw_sheet(lv_layer_t *l)
{
    const sd_game_t *g = &U.g;
    const sd_player_t *p = &g->p[U.sel];
    char buf[48];
    ground(l);
    fill(l, 0, 0, UI_W - 1, 132, T->seat[U.sel], 0);
    snprintf(buf, sizeof(buf), S_SEAT_FMT, U.sel + 1);
    text(l, U.f_title, T->seat_ink[U.sel], LV_OPA_COVER, CX, 84, buf);
    text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 152, S_POISON);
    if (g->rules.poison_lethal)
        snprintf(buf, sizeof(buf), S_SHEET_POISON_FMT, sd_counter(p, SD_POISON), g->rules.poison_lethal);
    else
        snprintf(buf, sizeof(buf), "%d", sd_counter(p, SD_POISON));
    text(l, U.f_hub, T->ink, LV_OPA_COVER, CX, 182, buf);
    if (g->n_players > 1 && g->rules.cmd_lethal) {
        /* one column per source seat: seat label in its hue over the tally; "/ 21" once within 5 of lethal */
        text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 272, S_CMD);
        int cols = g->n_players - 1, k = 0;
        for (int s = 0; s < g->n_players; s++) {
            if (s == U.sel)
                continue;
            int x = CX + (k++ - (cols - 1) / 2.f) * 84;
            snprintf(buf, sizeof(buf), S_SEAT_FMT, s + 1);
            text(l, U.f_small, T->seat[s], LV_OPA_COVER, x, 294, buf);
            if (p->cmd[s][0] + 5 >= g->rules.cmd_lethal)
                snprintf(buf, sizeof(buf), "%d / %d", p->cmd[s][0], g->rules.cmd_lethal);
            else
                snprintf(buf, sizeof(buf), "%d", p->cmd[s][0]);
            text(l, U.f_row, T->ink, LV_OPA_COVER, x, 320, buf);
        }
    }
    if (g->has_pending && g->pending.player == U.sel && g->pending.kind == SD_POISON) {
        const int y1 = 212, y2 = 244;
        char d[12];
        fmt_signed(d, sizeof(d), g->pending.amount);
        snprintf(buf, sizeof(buf), "%s  = %d", d, view_poison(g));
        band(l, y1, y2, buf);
    }
}

static void draw_over(lv_layer_t *l)
{
    int w = sd_game_winner(&U.g);
    uint32_t field = w >= 0 ? T->seat[w] : T->ground, ink = w >= 0 ? T->seat_ink[w] : T->ink;
    char buf[24];
    if (w >= 0)
        fill(l, 0, 0, UI_W - 1, UI_H - 1, field, 0);
    else
        ground(l);
    if (w >= 0)
        snprintf(buf, sizeof(buf), S_WINS_FMT, w + 1);
    else
        snprintf(buf, sizeof(buf), S_DEFEAT);
    text(l, title_font(buf), ink, LV_OPA_COVER, CX, 112, buf);
    text(l, U.f_label, ink, LV_OPA_COVER, CX, 158, S_GAME_OVER);
    outline(l, 70, 200, 290, 246, ink);
    text(l, U.f_row, ink, LV_OPA_COVER, CX, 223, S_MENU_UNDO);
    fill(l, 70, 258, 290, 304, ink, 0);
    text(l, U.f_row, field, LV_OPA_COVER, CX, 281, S_MENU_NEW);
}

/* ---------- luminous theme: menus ---------- */

static void draw_menu_light(lv_layer_t *l)
{
    char buf[64];
    ground(l);
    for (int i = 0; i < MENU_ROWS; i++) {
        int y = menu_row_y(i);
        if (i == U.menu_focus) { /* focus = a lit glass pill, kept inside the round dial */
            int dy = abs(y - CY) + 24, hw = (int)sqrtf((float)(R * R - dy * dy)) - 8;
            hw = hw > 144 ? 144 : hw;
            pill(l, CX - hw, y - 24, CX + hw, y + 23, true);
        }
        text(l, U.f_row, T->ink, i == U.menu_focus ? LV_OPA_COVER : LV_OPA_70, CX, y, menu_label(i, buf, sizeof(buf)));
    }
}

static void draw_log_light(lv_layer_t *l)
{
    ground(l);
    text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 48, S_MENU_LOG);
    line(l, 120, 70, 240, 70, T->ink, LV_OPA_40);
    if (!U.g.log_len)
        text(l, U.f_row, T->ink, LV_OPA_70, CX, CY, S_LOG_EMPTY);
    for (int k = 0; k < LOG_ROWS; k++) {
        int idx = U.g.log_len - 1 - U.log_scroll - k;
        if (idx < 0)
            break;
        const sd_action_t *a = &U.g.log[idx];
        int y = 104 + k * 44;
        fx_halo(l, 77, y, 14, log_hue(a), LV_OPA_COVER); /* a point of the seat's light; the text names it */
        log_row(l, a, y);
    }
    text(l, U.f_small, T->ink, LV_OPA_70, CX, 326, S_TAP_BACK);
}

static void draw_sheet_light(lv_layer_t *l)
{
    const sd_game_t *g = &U.g;
    const sd_player_t *p = &g->p[U.sel];
    char buf[48];
    ground(l);
    knob_ring(l);
    fx_halo(l, CX, 84, 96, T->seat[U.sel], LV_OPA_30);
    snprintf(buf, sizeof(buf), S_SEAT_FMT, U.sel + 1);
    text(l, U.f_title, T->ink, LV_OPA_COVER, CX, 84, buf);
    text(l, U.f_label, T->seat[U.sel], LV_OPA_COVER, CX, 152, S_POISON);
    if (g->rules.poison_lethal)
        snprintf(buf, sizeof(buf), S_SHEET_POISON_FMT, sd_counter(p, SD_POISON), g->rules.poison_lethal);
    else
        snprintf(buf, sizeof(buf), "%d", sd_counter(p, SD_POISON));
    text(l, U.f_hub, T->ink, LV_OPA_COVER, CX, 182, buf);
    if (g->n_players > 1 && g->rules.cmd_lethal) {
        /* one column per source seat: seat label in its hue over the tally; "/ 21" once within 5 of lethal */
        text(l, U.f_label, T->seat[U.sel], LV_OPA_COVER, CX, 268, S_CMD);
        int cols = g->n_players - 1, k = 0;
        for (int s = 0; s < g->n_players; s++) {
            if (s == U.sel)
                continue;
            int x = CX + (k++ - (cols - 1) / 2.f) * 78;
            snprintf(buf, sizeof(buf), S_SEAT_FMT, s + 1);
            text(l, U.f_small, T->seat[s], LV_OPA_COVER, x, 290, buf);
            if (p->cmd[s][0] + 5 >= g->rules.cmd_lethal)
                snprintf(buf, sizeof(buf), "%d / %d", p->cmd[s][0], g->rules.cmd_lethal);
            else
                snprintf(buf, sizeof(buf), "%d", p->cmd[s][0]);
            text(l, U.f_row, T->ink, LV_OPA_COVER, x, 314, buf);
        }
    }
    if (g->has_pending && g->pending.player == U.sel && g->pending.kind == SD_POISON) {
        char d[12];
        fmt_signed(d, sizeof(d), g->pending.amount);
        snprintf(buf, sizeof(buf), "%s  = %d", d, view_poison(g));
        pill(l, 106, 208, 254, 246, false);
        text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 227, buf);
        arc(l, KNOB_R + 2, -90.f, -90.f + 360.f * pending_progress(), 5, T->ink, LV_OPA_COVER);
    }
}

static void draw_over_light(lv_layer_t *l)
{
    int w = sd_game_winner(&U.g);
    char buf[24];
    ground(l);
    knob_ring(l);
    if (w >= 0) { /* the winner's light fills the well */
        fx_halo(l, CX, 120, 170, T->seat[w], LV_OPA_30);
        snprintf(buf, sizeof(buf), S_WINS_FMT, w + 1);
    } else
        snprintf(buf, sizeof(buf), S_DEFEAT);
    text(l, title_font(buf), T->ink, w >= 0 ? LV_OPA_COVER : LV_OPA_60, CX, 112, buf);
    text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 158, S_GAME_OVER);
    pill(l, 70, 200, 290, 246, false);
    text(l, U.f_row, T->ink, LV_OPA_COVER, CX, 223, S_MENU_UNDO);
    pill(l, 70, 258, 290, 304, true);
    text(l, U.f_row, T->ink, LV_OPA_COVER, CX, 281, S_MENU_NEW);
}

/* ---------- logo: boot screen (both themes) and the luminous home ---------- */

extern const lv_image_dsc_t ui_logo_a8, ui_logo_small_a8;         /* ui/assets/logo2a8.py */
extern const lv_image_dsc_t ui_wordmark_a8, ui_wordmark_small_a8; /* ui/assets/wordmark2a8.py: the product name */

static void draw_boot(lv_layer_t *l)
{
    ground(l);
    if (T->light)
        fx_halo(l, CX, 150, 150, T->ink, LV_OPA_20); /* the well of light the logo rises from */
    fx_image(l, &ui_logo_a8, CX, 142, T->ink, LV_OPA_COVER);
    fx_image(l, &ui_wordmark_a8, CX, 286, T->ink, LV_OPA_COVER);
}

/* Button of both themes: a glass pill (lit when primary) or the booster's flat block. Returns the ink to write on it. */
static uint32_t button(lv_layer_t *l, int x1, int y1, int x2, int y2, bool primary)
{
    if (T->light) {
        pill(l, x1, y1, x2, y2, primary);
        return T->ink;
    }
    if (primary)
        fill(l, x1, y1, x2, y2, T->ink, 0);
    else
        outline(l, x1, y1, x2, y2, T->ink);
    return primary ? T->ground : T->ink;
}

/* Home, radial: START (new game) at the centre, REPRENDRE below when a game is saved. Settings, update, host and
 * join are shown dimmed and do nothing yet (plan #2 and the connectivity lots give them a destination). */
extern const lv_image_dsc_t ui_home_icons_a8[4];
#define NEW_Y 160
#define NEW_R 64
#define RESUME_X1 116
#define RESUME_Y1 276
#define RESUME_X2 244
#define RESUME_Y2 326
static void draw_home(lv_layer_t *l)
{
    static const struct { /* order = ui_home_icons_a8[] (ui/assets/icons2a8.py) */
        int x, y, r;
        const char *label;
    } off[] = {{92, 96, 24, NULL}, {268, 96, 24, NULL}, {76, 206, 34, S_HOST}, {284, 206, 34, S_JOIN}};
    char buf[24];
    ground(l);
    fx_image(l, &ui_wordmark_small_a8, CX, 46, T->ink, LV_OPA_COVER);
    for (size_t i = 0; i < sizeof(off) / sizeof(off[0]); i++) {
        button(l, off[i].x - off[i].r, off[i].y - off[i].r, off[i].x + off[i].r, off[i].y + off[i].r, false);
        fx_image(l, &ui_home_icons_a8[i], off[i].x, off[i].y, T->ink, LV_OPA_40);
        if (off[i].label)
            text(l, U.f_small, T->ink, LV_OPA_40, off[i].x, off[i].y + off[i].r + 16, off[i].label);
    }
    /* with a saved game REPRENDRE is the lit action: a new game would replace it */
    if (T->light && !U.has_resume)
        fx_ring(l, CX, NEW_Y, NEW_R, T->ink, LV_OPA_70);
    uint32_t ink = button(l, CX - NEW_R, NEW_Y - NEW_R, CX + NEW_R, NEW_Y + NEW_R, !U.has_resume);
    fx_image(l, &ui_logo_small_a8, CX, NEW_Y - 12, ink, LV_OPA_COVER); /* the product's mark starts a game */
    text(l, U.f_row, ink, LV_OPA_COVER, CX, NEW_Y + 38, S_START);
    if (U.has_resume) {
        ink = button(l, RESUME_X1, RESUME_Y1, RESUME_X2, RESUME_Y2, true);
        text(l, U.f_small, ink, LV_OPA_COVER, CX, RESUME_Y1 + 15, S_RESUME);
        snprintf(buf, sizeof(buf), S_RESUME_SUB_FMT, U.resume.turn, U.resume.n_players);
        text(l, U.f_small, ink, LV_OPA_COVER, CX, RESUME_Y1 + 36, buf);
    }
}

/* New game, step 1: the format, step 2: the table size. The knob (or a tap on either side) changes the value. */
#define NEW_BTN_Y1 262
#define NEW_BTN_Y2 310
static void players_step(int d) /* SOLO, then the sizes the format allows */
{
    const sd_profile_t *p = &U.profiles[U.profile];
    int n = p->players_max - p->players_min + 2;
    int i = U.players == 1 ? 0 : U.players - p->players_min + 1;
    i = ((i + d) % n + n) % n;
    U.players = i ? p->players_min + i - 1 : 1;
}

static void draw_new(lv_layer_t *l)
{
    const sd_profile_t *p = &U.profiles[U.profile];
    char buf[64], name[24];
    ground(l);
    if (T->light)
        knob_ring(l);
    snprintf(buf, sizeof(buf), "%s", p->family);
    upper(buf);
    text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 60, U.new_step == NEW_PLAYERS ? S_PLAYERS : buf[0] ? buf : S_FORMAT);
    snprintf(name, sizeof(name), "%s", p->name);
    upper(name);
    text(l, title_font(name), T->ink, LV_OPA_COVER, CX, 112, name);
    /* only the rules the format has: no commander damage, no mention of it */
    int n = snprintf(buf, sizeof(buf), S_LIFE_FMT, p->rules.start_life);
    if (p->rules.cmd_lethal)
        n += snprintf(buf + n, sizeof(buf) - (size_t)n, S_DOT S_CMD " %d", p->rules.cmd_lethal);
    if (p->rules.poison_lethal)
        snprintf(buf + n, sizeof(buf) - (size_t)n, S_DOT S_POISON " %d", p->rules.poison_lethal);
    text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 154, buf);
    if (U.new_step == NEW_FORMAT) {
        for (int i = 0; i < U.n_profiles; i++) { /* where we are in the list */
            int x = CX - (U.n_profiles - 1) * 7 + i * 14;
            fill(l, x - 3, 207, x + 3, 213, T->ink, T->light ? LV_RADIUS_CIRCLE : 0);
            if (i != U.profile)
                fill(l, x - 2, 208, x + 2, 212, T->ground, T->light ? LV_RADIUS_CIRCLE : 0);
        }
    } else {
        for (int i = 0; i < p->players_max; i++) { /* seats in play are lit, the others are empty */
            int x = CX - (p->players_max - 1) * 22 + i * 44, y = 204;
            bool on = i < U.players;
            lv_draw_rect_dsc_t d;
            lv_draw_rect_dsc_init(&d);
            d.radius = T->light ? LV_RADIUS_CIRCLE : 0;
            d.bg_color = C(T->seat[i]);
            d.bg_opa = !on ? LV_OPA_TRANSP : T->light ? 50 : LV_OPA_COVER;
            d.border_color = C(on || !T->light ? T->seat[i] : T->ink);
            d.border_opa = on || !T->light ? LV_OPA_COVER : LV_OPA_30;
            d.border_width = 2;
            lv_area_t a = {x - 18, y - 18, x + 18, y + 18};
            if (on && T->light)
                fx_halo(l, x, y, 30, T->seat[i], LV_OPA_20);
            lv_draw_rect(l, &d, &a);
            snprintf(buf, sizeof(buf), S_SEAT_FMT, i + 1);
            text(l, U.f_small, T->light ? T->ink : on ? T->seat_ink[i] : T->seat[i],
                 on || !T->light ? LV_OPA_COVER : LV_OPA_40, x, y, buf);
        }
        if (U.players == 1)
            snprintf(buf, sizeof(buf), S_SOLO);
        else
            snprintf(buf, sizeof(buf), S_PLAYERS_FMT, U.players, "S");
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 240, buf);
    }
    for (int k = -1; k <= 1; k += 2) /* chevrons: a tap on either side steps like the knob */
        text(l, U.f_row, T->ink, LV_OPA_60, CX + k * 124, 206, k < 0 ? "\xE2\x80\xB9" : "\xE2\x80\xBA");
    button(l, 66, NEW_BTN_Y1 + 2, 110, NEW_BTN_Y2 - 2, false); /* back */
    fx_icon(l, FX_ICON_UNDO, 88, (NEW_BTN_Y1 + NEW_BTN_Y2) / 2, T->ink, LV_OPA_COVER);
    uint32_t ink = button(l, 124, NEW_BTN_Y1, 264, NEW_BTN_Y2, true);
    text(l, U.f_row, ink, LV_OPA_COVER, 194, (NEW_BTN_Y1 + NEW_BTN_Y2) / 2, U.new_step == NEW_FORMAT ? S_NEXT : S_PLAY);
}

static void markers_btn(lv_layer_t *l)
{
    int x, y;
    if (!markers_btn_shown())
        return;
    markers_btn_xy(&x, &y);
    uint32_t ink = button(l, x - MARKERS_BTN_R, y - MARKERS_BTN_R, x + MARKERS_BTN_R, y + MARKERS_BTN_R, false);
    text(l, U.f_small, ink, LV_OPA_COVER, x, y, "\xE2\x80\xA2\xE2\x80\xA2\xE2\x80\xA2"); /* three dots: "more" */
}

/* Markers panel of the selected seat: eleven chips on a ring (seven counters, then ascension, monarch, initiative,
 * day/night). A tap focuses a chip, the knob changes it; a state chip also flips on its tap. The centre names the
 * focused marker, shows its value, and takes the tap that goes back to the game. */
#define MARKER_RING 132
#define MARKER_CHIP 23
static void marker_xy(int i, int *x, int *y)
{
    float a = (-90.f + i * 360.f / N_MARKERS) * (float)M_PI / 180.f;
    *x = CX + (int)lroundf(MARKER_RING * cosf(a));
    *y = CY + (int)lroundf(MARKER_RING * sinf(a));
}

/* is the marker "on" for the selected seat: a counter above zero, a state it holds */
static bool marker_on(int i)
{
    const sd_game_t *g = &U.g;
    switch (SD_POISON + i) {
    case SD_ASCEND: return g->p[U.sel].ascended;
    case SD_MONARCH: return g->monarch == U.sel;
    case SD_INITIATIVE: return g->initiative == U.sel;
    case SD_DAYNIGHT: return g->daynight != SD_DN_NONE;
    default: return g->p[U.sel].counter[i] > 0;
    }
}

/* Knob step (or the tap on a state chip, as +1 / -1) on the focused marker. */
static void marker_change(int d)
{
    sd_game_t *g = &U.g;
    sd_kind_t kind = (sd_kind_t)(SD_POISON + U.marker);
    if (kind < SD_ASCEND) {
        adjust(U.sel, kind, U.sel, d);
        return;
    }
    uint8_t v;
    if (kind == SD_ASCEND) {
        if (g->p[U.sel].ascended == (d > 0))
            return;
        v = U.sel;
    } else if (kind == SD_DAYNIGHT)
        v = (uint8_t)(((g->daynight + d) % 3 + 3) % 3);
    else
        v = d > 0 ? U.sel : SD_NOBODY; /* clockwise: this seat takes it; back: nobody holds it */
    bool lost[SD_MAX_PLAYERS];
    snapshot_lost(lost);
    if (sd_game_set(g, kind, v))
        committed(lost);
}

/* Owner-supplied icon of a marker chip (ui_marker_icons_a8[], ui/assets/icons2a8.py), NULL while it has none: the
 * chip then shows its 3-letter label. Day/night shows the moon at night, the sun otherwise. */
extern const lv_image_dsc_t ui_marker_icons_a8[4]; /* energy, rad, day, night */
static const lv_image_dsc_t *marker_icon(int i)
{
    switch (SD_POISON + i) {
    case SD_ENERGY: return &ui_marker_icons_a8[0];
    case SD_RAD: return &ui_marker_icons_a8[1];
    case SD_DAYNIGHT: return &ui_marker_icons_a8[U.g.daynight == SD_DN_NIGHT ? 3 : 2];
    default: return NULL;
    }
}

static void draw_markers(lv_layer_t *l)
{
    const sd_game_t *g = &U.g;
    const sd_player_t *p = &g->p[U.sel];
    sd_kind_t kind = (sd_kind_t)(SD_POISON + U.marker);
    char buf[32], who[8];
    ground(l);
    if (T->light)
        knob_ring(l);
    for (int i = 0; i < N_MARKERS; i++) {
        int x, y;
        marker_xy(i, &x, &y);
        bool focus = i == U.marker, on = marker_on(i);
        /* seat hue = this seat has it (the centre gives the value); focus = a lit chip, or for a bare icon white light
         * behind it (Lumineux) or a rule under it (Booster) */
        if (focus && marker_icon(i) && T->light)
            fx_halo(l, x, y, 40, 0xFFFFFF, LV_OPA_50);
        else if (on && T->light)
            fx_halo(l, x, y, 40, T->seat[U.sel], LV_OPA_30);
        lv_opa_t o = on || focus ? LV_OPA_COVER : LV_OPA_70;
        if (marker_icon(i)) { /* an icon stands on its own, at the size of the chip it replaces */
            fx_image(l, marker_icon(i), x, y, on ? T->seat[U.sel] : T->ink, o);
            if (focus && !T->light)
                line(l, x - 16, y + MARKER_CHIP + 4, x + 16, y + MARKER_CHIP + 4, T->ink, LV_OPA_COVER);
        } else {
            uint32_t ink = button(l, x - MARKER_CHIP, y - MARKER_CHIP, x + MARKER_CHIP, y + MARKER_CHIP, focus);
            text(l, U.f_small, on && (T->light || !focus) ? T->seat[U.sel] : ink, o, x, y, MARKER_ABBR[i]);
        }
    }
    snprintf(buf, sizeof(buf), S_SEAT_FMT, U.sel + 1);
    text(l, U.f_label, T->seat[U.sel], LV_OPA_COVER, CX, 104, buf);
    text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 134, MARKER_NAME[U.marker]);
    const char *v = buf;
    if (kind == SD_POISON && g->rules.poison_lethal)
        snprintf(buf, sizeof(buf), S_SHEET_POISON_FMT, sd_counter(p, SD_POISON), g->rules.poison_lethal);
    else if (kind < SD_ASCEND)
        snprintf(buf, sizeof(buf), "%d", sd_counter(p, kind));
    else if (kind == SD_ASCEND)
        v = p->ascended ? S_YES : S_NO;
    else if (kind == SD_DAYNIGHT)
        v = g->daynight == SD_DN_DAY ? S_DAY : g->daynight == SD_DN_NIGHT ? S_NIGHT : S_NONE;
    else
        v = holder(who, sizeof(who), kind == SD_MONARCH ? g->monarch : g->initiative);
    text(l, U.f_hub, T->ink, LV_OPA_COVER, CX, 180, v);
    if (g->has_pending && g->pending.player == U.sel && g->pending.kind == kind) {
        char d[12];
        sd_player_t view = sd_game_view(g, U.sel);
        fmt_signed(d, sizeof(d), g->pending.amount);
        snprintf(buf, sizeof(buf), "%s  = %d", d, sd_counter(&view, kind));
        button(l, 116, 210, 244, 246, false);
        text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 228, buf);
        if (T->light)
            arc(l, KNOB_R + 2, -90.f, -90.f + 360.f * pending_progress(), 5, T->ink, LV_OPA_COVER);
    } else
        text(l, U.f_small, T->ink, LV_OPA_70, CX, 232, S_MENU_BACK);
}

static void draw_cb(lv_event_t *e)
{
    lv_layer_t *l = lv_event_get_layer(e);
    switch (U.scr) {
    case UI_SCR_BOOT: draw_boot(l); break;
    case UI_SCR_HOME: draw_home(l); break;
    case UI_SCR_NEW: draw_new(l); break;
    case UI_SCR_MARKERS: draw_markers(l); break;
    case UI_SCR_GAME:
        if (U.g.n_players == 1)
            T->light ? draw_solo_light(l) : draw_solo(l);
        else
            T->light ? draw_arena_light(l) : draw_arena(l);
        break;
    case UI_SCR_MENU: T->light ? draw_menu_light(l) : draw_menu(l); break;
    case UI_SCR_LOG: T->light ? draw_log_light(l) : draw_log(l); break;
    case UI_SCR_SHEET: T->light ? draw_sheet_light(l) : draw_sheet(l); break;
    case UI_SCR_OVER: T->light ? draw_over_light(l) : draw_over(l); break;
    }
}

/* ---------- public API ---------- */

void ui_set_theme(int theme, int ground_idx)
{
    TH = *THEMES[theme >= 0 && theme < UI_THEME_COUNT ? theme : UI_THEME_DEFAULT];
    if (TH.ground_pick) {
        TH.ground = ui_grounds[ground_idx >= 0 && ground_idx < UI_GROUND_COUNT ? ground_idx : 0];
        TH.ground_rim = ui_ground_rim(TH.ground);
    }
    T = &TH;
    /* ponytail: both themes use the same fonts, so they are not rebuilt; rebuild here if a theme brings its own */
    if (U.dial) {
        lv_obj_set_style_bg_color(lv_screen_active(), C(T->ground_rim), 0);
        redraw();
    }
}

void ui_init(const ui_hooks_t *hooks, const sd_profile_t *profiles, int n_profiles, const sd_game_t *resume)
{
    memset(&U, 0, sizeof(U));
    if (hooks)
        U.hooks = *hooks;
    U.profiles = profiles;
    U.n_profiles = n_profiles;
    U.players = n_profiles ? profiles[0].players : 1;
    if (resume) {
        U.resume = *resume;
        U.has_resume = true;
    }
    /* kerning off: GPOS is hidden from stb (ui/assets/strip_gpos.py); neither font has a kern table */
#define TTF(data, len, px) lv_tiny_ttf_create_data_ex(data, len, px, LV_FONT_KERNING_NONE, 64)
    const uint8_t *dsp = T->display_ttf, *lbl = T->label_ttf;
    size_t dl = (size_t)(T->display_ttf_end - dsp), ll = (size_t)(T->label_ttf_end - lbl);
    U.f_solo = TTF(dsp, dl, 170);
    U.f_arena[2] = TTF(dsp, dl, 112);
    U.f_arena[3] = TTF(dsp, dl, 84);
    U.f_arena[4] = TTF(dsp, dl, 72);
    U.f_hub = TTF(dsp, dl, 48);
    U.f_title = TTF(dsp, dl, 56);
    U.f_name = TTF(dsp, dl, 32);
    U.f_row = TTF(lbl, ll, 30);
    U.f_label = TTF(lbl, ll, 24);
    U.f_small = TTF(lbl, ll, 20);

    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, C(T->ground_rim), 0);
    U.dial = lv_obj_create(scr);
    lv_obj_remove_style_all(U.dial);
    lv_obj_set_size(U.dial, UI_W, UI_H);
    lv_obj_add_event_cb(U.dial, draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    U.scr = UI_SCR_BOOT;
    redraw();
}

void ui_knob(int detents, uint32_t now)
{
    U.now = now;
    if (!detents)
        return;
    switch (U.scr) {
    case UI_SCR_BOOT: U.scr = UI_SCR_HOME; break; /* any input skips the logo */
    case UI_SCR_HOME: return; /* two actions, both a tap away: the knob has nothing to drive here yet */
    case UI_SCR_NEW:
        if (U.new_step == NEW_FORMAT) {
            U.profile = ((U.profile + detents) % U.n_profiles + U.n_profiles) % U.n_profiles;
            U.players = U.profiles[U.profile].players;
        } else
            players_step(detents);
        haptic(UI_HAPTIC_DETENT);
        break;
    case UI_SCR_GAME: {
        uint16_t before = U.g.log_len;
        /* the knob keeps feeding the pending delta on the selected seat; ccw always lowers life */
        if (U.g.has_pending && U.g.pending.player == U.sel)
            adjust(U.sel, (sd_kind_t)U.g.pending.kind, U.g.pending.src,
                   U.g.pending.kind == SD_CMD ? -detents : detents);
        else
            adjust(U.sel, SD_LIFE, U.sel, detents);
        if (U.g.log_len == before && U.scr == UI_SCR_GAME) { /* nothing committed: only the hub changed */
            redraw_hub();
            return;
        }
        break;
    }
    case UI_SCR_SHEET: adjust(U.sel, SD_POISON, U.sel, detents); break;
    case UI_SCR_MARKERS: marker_change(detents); break;
    case UI_SCR_MENU:
        U.menu_focus = ((U.menu_focus + detents) % MENU_ROWS + MENU_ROWS) % MENU_ROWS;
        haptic(UI_HAPTIC_DETENT);
        break;
    case UI_SCR_LOG: {
        int max = U.g.log_len > LOG_ROWS ? U.g.log_len - LOG_ROWS : 0;
        U.log_scroll += detents;
        U.log_scroll = U.log_scroll < 0 ? 0 : U.log_scroll > max ? max : U.log_scroll;
        haptic(UI_HAPTIC_DETENT);
        break;
    }
    case UI_SCR_OVER: break;
    }
    redraw();
}

static void menu_activate(int row)
{
    switch (row) {
    case MENU_UNDO:
        undo();
        U.scr = UI_SCR_GAME;
        break;
    case MENU_LOG:
        U.log_scroll = 0;
        U.scr = UI_SCR_LOG;
        break;
    case MENU_SHEET: U.scr = UI_SCR_SHEET; break;
    case MENU_NEW: go_home(); break;
    default: U.scr = UI_SCR_GAME; break;
    }
}

void ui_tap(int x, int y, uint32_t now)
{
    U.now = now;
    switch (U.scr) {
    case UI_SCR_BOOT: U.scr = UI_SCR_HOME; break;
    case UI_SCR_HOME:
        if ((x - CX) * (x - CX) + (y - NEW_Y) * (y - NEW_Y) <= NEW_R * NEW_R && U.n_profiles) {
            U.new_step = NEW_FORMAT;
            U.scr = UI_SCR_NEW;
        } else if (U.has_resume && x >= RESUME_X1 && x <= RESUME_X2 && y >= RESUME_Y1 && y <= RESUME_Y2)
            start_game(&U.resume);
        break;
    case UI_SCR_NEW:
        if (y >= NEW_BTN_Y1 - 8 && y <= NEW_BTN_Y2 + 8) {
            if (x >= 124 && x <= 264) {
                if (U.new_step == NEW_PLAYERS)
                    start_game(NULL);
                else
                    U.new_step = NEW_PLAYERS;
            } else if (x >= 58 && x < 118) { /* back: one step, then home */
                if (U.new_step == NEW_PLAYERS)
                    U.new_step = NEW_FORMAT;
                else
                    U.scr = UI_SCR_HOME;
            }
        } else if (y < 240 && (x < 110 || x >= 250)) {
            ui_knob(x < 110 ? -1 : 1, now);
            return;
        }
        break;
    case UI_SCR_GAME:
        if (markers_btn_hit(x, y)) {
            flush();
            if (U.scr == UI_SCR_GAME) {
                U.marker = 0;
                U.scr = UI_SCR_MARKERS;
            }
        } else if (U.g.n_players == 1) {
            if (undo_pill() && y >= 276 && y < 324 && x >= 100 && x < 260)
                undo();
            else if (x < 110 || x >= 250)
                ui_knob(x < 110 ? -1 : 1, now);
        } else if (in_hub(x, y)) {
            if (sd_game_pending_toggle_cmd(&U.g)) { /* attack <-> commander damage */
                U.g.pending_since = now;
                haptic(UI_HAPTIC_DETENT);
            } else if (U.g.has_pending)
                flush(); /* tapping the seal commits now */
            else if (undo_pill())
                undo();
            else
                next_turn();
        } else {
            int s = seat_at(x, y);
            if (s >= 0 && s != U.sel) {
                flush();
                U.sel = (uint8_t)s;
            } else if (s >= 0) { /* selected seat: - / + on either side of the number, in the seat's own frame */
                float lx = seat_local_x(s, x, y);
                if (lx < -PM_TAP || lx > PM_TAP)
                    ui_knob(lx < 0 ? -1 : 1, now);
            }
        }
        break;
    case UI_SCR_MENU:
        for (int i = 0; i < MENU_ROWS; i++)
            if (y >= menu_row_y(i) - 26 && y < menu_row_y(i) + 26) {
                U.menu_focus = i;
                menu_activate(i);
            }
        break;
    case UI_SCR_LOG: U.scr = UI_SCR_MENU; break;
    case UI_SCR_SHEET:
        flush();
        if (U.scr == UI_SCR_SHEET)
            U.scr = UI_SCR_GAME;
        break;
    case UI_SCR_MARKERS: {
        int hit = -1;
        for (int i = 0; i < N_MARKERS && hit < 0; i++) {
            int cx, cy;
            marker_xy(i, &cx, &cy);
            if ((x - cx) * (x - cx) + (y - cy) * (y - cy) <= 30 * 30)
                hit = i;
        }
        flush(); /* a pending counter is committed before the focus moves or the panel closes */
        if (U.scr != UI_SCR_MARKERS)
            break; /* that commit ended the game */
        if (hit < 0) {
            if ((x - CX) * (x - CX) + (y - CY) * (y - CY) <= 100 * 100)
                U.scr = UI_SCR_GAME;
        } else {
            U.marker = hit;
            if (SD_POISON + hit >= SD_ASCEND) /* a state flips on its own chip: one gesture */
                marker_change(marker_on(hit) && SD_POISON + hit != SD_DAYNIGHT ? -1 : 1);
        }
        break;
    }
    case UI_SCR_OVER:
        if (y >= 196 && y < 252) {
            undo();
            U.scr = sd_game_over(&U.g) ? UI_SCR_OVER : UI_SCR_GAME;
        } else if (y >= 252 && y < 310)
            go_home();
        break;
    }
    redraw();
}

void ui_long_press(int x, int y, uint32_t now)
{
    (void)x;
    (void)y;
    U.now = now;
    if (U.scr != UI_SCR_GAME)
        return;
    flush();
    if (U.scr == UI_SCR_GAME) {
        U.menu_focus = MENU_UNDO;
        U.scr = UI_SCR_MENU;
    }
    redraw();
}

void ui_drag(int x0, int y0, int x1, int y1, uint32_t now)
{
    U.now = now;
    if (U.scr != UI_SCR_GAME || U.g.n_players < 2)
        return;
    int a = seat_at(x0, y0), b = seat_at(x1, y1);
    if (a < 0 || b < 0 || a == b)
        return;
    U.sel = (uint8_t)b; /* A attacks B: combat damage, the knob sets the amount, a hub tap makes it commander damage */
    adjust((uint8_t)b, SD_LIFE, (uint8_t)a, -1);
    redraw();
}

void ui_tick(uint32_t now)
{
    U.now = now;
    if (U.scr == UI_SCR_BOOT) { /* the delay starts at the first tick: ui_init() has no clock */
        if (!U.boot_until)
            U.boot_until = now + UI_BOOT_MS;
        else if (now >= U.boot_until) {
            U.scr = UI_SCR_HOME;
            redraw();
        }
        return;
    }
    if (U.undo_until && now >= U.undo_until) {
        U.undo_until = 0;
        if (T->light)
            redraw(); /* the lit pill's halo reaches past the hub box */
        else
            redraw_hub();
    }
    if (!U.g.has_pending)
        return;
    bool lost[SD_MAX_PLAYERS];
    snapshot_lost(lost);
    if (sd_game_tick(&U.g, now)) {
        committed(lost);
        redraw();
    } else
        redraw_hub();
}

ui_screen_t ui_screen(void) { return U.scr; }
const sd_game_t *ui_game(void) { return &U.g; }
int ui_selected(void) { return U.sel; }
