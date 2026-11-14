#include "ui.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fx.h"
#include "lvgl.h"
#include "settings.h"
#include "strings.h"
#include "theme.h"

#define CX 180
#define CY 180
#define R 180
#define HUB_R 48
#define MINUS "\xE2\x88\x92" /* U+2212 */
#define SEP " \xE2\x80\xBA "  /* U+203A, "J1 › J3" */
#define DOT " · "
#define NONE "—"

/* format-name font, embedded by the build from the git-ignored ui/assets/licensed/ (ui/name_font.cmake) */
extern const uint8_t _binary_Original_Motion_ttf_start[], _binary_Original_Motion_ttf_end[];
#define MENU_ROWS 6
#define LOG_ROWS 5
#define UNDO_PILL_MS 4000
#define PM_TAP 26 /* |local x| beyond which a tap on the selected seat is - or + */

enum { NEW_FORMAT, NEW_PLAYERS, NEW_VIEW }; /* the steps of UI_SCR_NEW; the last one only with several players */
enum { HOME_START, HOME_RESUME, HOME_SETTINGS, HOME_N }; /* what the knob focuses on the home screen */
enum { MENU_UNDO, MENU_LOG, MENU_SHEET, MENU_NEW, MENU_SETTINGS, MENU_BACK };

static const ui_theme_t *const THEMES[UI_THEME_COUNT] = {[UI_THEME_BOOSTER] = &ui_theme_booster,
                                                         [UI_THEME_LUMINEUX] = &ui_theme_lumineux};
static ui_theme_t TH; /* active theme, with the user's ground applied */
static sd_settings_t S; /* the settings, see ui_set_settings() */
static bool s_ready;
static const char *tr(int id) { return ui_langs[S.v[SD_S_LANG] % UI_LANG_COUNT]->str[id]; }
#define TR(id) tr(STR_##id)
static const ui_theme_t *T = &ui_theme_lumineux; /* == *THEMES[UI_THEME_DEFAULT] until ui_set_theme() */

static struct {
    ui_hooks_t hooks;
    const sd_profile_t *profiles;
    int n_profiles;
    int profile, players, new_step, home_focus;
    int marker; /* focused chip of the markers panel: kind - SD_POISON */
    bool has_resume;
    bool in_game;     /* U.g is the game being played (otherwise the one to change is U.resume, if any) */
    int sheet_src;    /* player sheet: the commander column the knob drives, -1 = poison */
    int16_t click_rest; /* knob detents not yet worth a point (SD_S_CLICKS) */
    bool dirty; /* a setting changed and the HAL has not been told yet */
    sd_game_t resume;
    sd_game_t g;
    char profile_name[24];
    uint8_t sel;
    ui_screen_t scr;
    int menu_focus, log_scroll;
    /* settings: the sections grid, then the list of one section */
    int set_focus, section, row, armed; /* armed: the action row waiting for its confirming tap, or -1 */
    bool set_from_game, editing;
    struct { /* text / PIN entry */
        char buf[32];
        uint8_t len, max, ch, purpose;
        bool wrong;
        ui_screen_t back;
    } ed;
    uint32_t now, undo_until, boot_until, seed;
    /* first-start assistant */
    int setup_step, cal; /* cal: detents counted by the knob step */
    bool setup_first;    /* run at power-on (ends on the home screen) rather than from the settings */
    uint8_t touched;     /* touch step: the targets hit so far, one bit each */
    lv_obj_t *dial;
    lv_font_t *f_solo, *f_arena[SD_MAX_PLAYERS + 1], *f_hub, *f_title, *f_name[2], *f_row, *f_label, *f_small;
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
#define MARKER_NAME (ui_langs[S.v[SD_S_LANG] % UI_LANG_COUNT]->str + STR_MK_POISON)
#define MARKER_ABBR (ui_langs[S.v[SD_S_LANG] % UI_LANG_COUNT]->str + STR_MKA_POISON)

static void appendf(char *buf, size_t n, const char *fmt, ...)
{
    size_t len = strlen(buf);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf + len, n - len, fmt, ap);
    va_end(ap);
}

/* "J3"; four names may be alive in one expression */
static const char *seat(int i)
{
    static char b[4][8];
    static int k;
    k = (k + 1) & 3;
    snprintf(b[k], sizeof(b[k]), TR(SEAT_FMT), i + 1);
    return b[k];
}

/* who a state names: "J3", or a dash for nobody */
static const char *holder(char *buf, size_t n, uint8_t who)
{
    if (who >= U.g.n_players)
        return NONE;
    snprintf(buf, n, TR(SEAT_FMT), who + 1);
    return buf;
}

static void fmt_action(char *buf, size_t n, const sd_action_t *a)
{
    char d[12];
    fmt_signed(d, sizeof(d), a->amount);
    const char *who = seat(a->player);
    if (a->kind == SD_TURN) {
        snprintf(buf, n, TR(TURN_FMT), a->turn);
        appendf(buf, n, "  %s" SEP "%s", who, seat(a->src));
    } else if (a->kind == SD_LIFE && a->src != a->player)
        snprintf(buf, n, "%s" SEP "%s  %s %s", seat(a->src), who, TR(LIFE), d);
    else if (a->kind == SD_LIFE)
        snprintf(buf, n, "%s  %s %s", who, TR(LIFE), d);
    else if (a->kind == SD_CMD || a->kind == SD_CMD2)
        snprintf(buf, n, "%s  %s %s %s", who, a->kind == SD_CMD ? TR(CMD) : TR(CMD2), seat(a->src), d);
    else if (a->kind < SD_ASCEND)
        snprintf(buf, n, "%s  %s %s", who, MARKER_NAME[a->kind - SD_POISON], d);
    else if (a->kind == SD_ASCEND)
        snprintf(buf, n, "%s  %s %s", who, MARKER_NAME[a->kind - SD_POISON], a->amount > 0 ? "+" : MINUS);
    else if (a->kind == SD_DAYNIGHT && a->player == SD_DN_NONE)
        snprintf(buf, n, "%s  " NONE, TR(MK_DAYNIGHT));
    else if (a->kind == SD_DAYNIGHT)
        snprintf(buf, n, "%s", a->player == SD_DN_DAY ? TR(DAY) : TR(NIGHT));
    else
        snprintf(buf, n, "%s  %s", MARKER_NAME[a->kind - SD_POISON], holder(d, sizeof(d), a->player));
}

/* What the undo pill takes back, in a few characters. */
static void fmt_undo(char *buf, size_t n, const sd_action_t *a)
{
    if (a->kind == SD_TURN) /* the hand-off it reverses */
        snprintf(buf, n, "%s" SEP "%s", seat(a->player), seat(a->src));
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
    uint32_t el = U.now - U.g.pending_since, ms = sd_commit_ms(&U.g);
    return el >= ms ? 1.f : (float)el / ms;
}

/* ---------- geometry ---------- */

/* One life total fills the dial: a game alone, or my life alone at a table of several (SD_VIEW_SOLO; seat 0 is mine). */
static bool solo(void) { return U.g.n_players == 1 || S.v[SD_S_VIEW] == SD_VIEW_SOLO; }

static float seat_span(void) { return 360.f / U.g.n_players; }
static float seat_angle(int i) { return 90.f + i * seat_span(); } /* J1 sits at the bottom, then clockwise */

static int seat_at(int x, int y)
{
    if (solo())
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
    if (U.scr != UI_SCR_GAME || solo()) {
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
static float seat_rot(int i) { return solo() || !S.v[SD_S_ROTATE_SEATS] ? 0.f : seat_angle(i) - 90.f; }
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
        sd_rules_t r = p->rules;
        sd_settings_rules(&S, &r);
        sd_game_new(&U.g, &r, (uint8_t)U.players);
        snprintf(U.g.format, sizeof(U.g.format), "%s", p->name);
        snprintf(U.profile_name, sizeof(U.profile_name), "%s", p->name);
        save();
    }
    upper(U.profile_name);
    U.sel = 0;
    U.click_rest = 0;
    U.in_game = true;
    U.has_resume = false;
    U.scr = sd_game_over(&U.g) ? UI_SCR_OVER : UI_SCR_GAME;
}

static void go_home(void)
{
    flush();
    U.resume = U.g; /* "Nouvelle partie" never destroys the game: it stays resumable */
    U.has_resume = true;
    U.in_game = false;
    U.home_focus = HOME_RESUME;
    U.scr = UI_SCR_HOME;
}

/* The game the settings act on: the one being played, else the one REPRENDRE would bring back. */
static sd_game_t *live_game(void) { return U.in_game ? &U.g : U.has_resume ? &U.resume : NULL; }

/* Settings that change the game under way reach it at once: the rule options and the commit delay. Starting life
 * and thresholds wait for the next game (or RÉINITIALISER LA PARTIE). */
static void apply_live(void)
{
    sd_game_t *g = live_game();
    sd_rules_t r = {0};
    sd_settings_rules(&S, &r);
    if (S.v[SD_S_VIEW] == SD_VIEW_SOLO)
        U.sel = 0;
    if (!g || (g->rules.opts == r.opts && g->rules.commit_ms == r.commit_ms))
        return;
    g->rules.opts = r.opts;
    g->rules.commit_ms = r.commit_ms;
    if (U.hooks.save)
        U.hooks.save(g);
}

/* Apply the settings and hand them to the HAL, if the user changed any since the last time. */
static void settings_save(void)
{
    if (!U.dirty)
        return;
    U.dirty = false;
    apply_live();
    if (U.hooks.save_settings)
        U.hooks.save_settings(&S);
}

/* Start the game under way again: same format, same table, today's settings. */
static void restart_game(void)
{
    sd_game_t *g = live_game();
    if (!g)
        return;
    sd_rules_t r = g->rules;
    char format[sizeof(g->format)];
    uint8_t n = g->n_players;
    memcpy(format, g->format, sizeof(format));
    for (int i = 0; i < U.n_profiles; i++)
        if (!strcmp(U.profiles[i].name, format))
            r = U.profiles[i].rules;
    sd_settings_rules(&S, &r);
    sd_game_new(g, &r, n);
    memcpy(g->format, format, sizeof(format));
    U.sel = 0;
    U.undo_until = 0;
    if (U.hooks.save)
        U.hooks.save(g);
    haptic(UI_HAPTIC_COMMIT);
}

/* ---------- screens: drawing ---------- */

/* Direct way into the markers panel: a three-dot button (not a "+", which already means +1 life), on the right in
 * Solo and in the right corner of the selected seat's rim in the arena (right as its player sees it). */
#define MARKERS_BTN_R 17
static void markers_btn_xy(int *x, int *y)
{
    if (solo()) {
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
            text(l, U.f_small, T->ground, LV_OPA_COVER, CX, CY - 14, TR(MENU_UNDO));
            fmt_undo(t, sizeof(t), a);
            text(l, U.f_row, T->ground, LV_OPA_COVER, CX, CY + 12, t);
            return;
        }
        /* at rest the hub is sealed (everything committed) and shows whose turn it is; tap = next turn */
        fill(l, CX - HUB_R, CY - HUB_R, CX + HUB_R, CY + HUB_R, T->ground, LV_RADIUS_CIRCLE);
        ring(l, HUB_R, 0.f, 360.f, T->ink);
        snprintf(t, sizeof(t), TR(TURN_FMT), g->turn);
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, CY - 20, t);
        snprintf(t, sizeof(t), TR(SEAT_FMT), g->active + 1);
        text(l, U.f_hub, T->seat[g->active], LV_OPA_COVER, CX, CY + 8, t);
        return;
    }
    char d[12], sub[24];
    fmt_signed(d, sizeof(d), g->pending.kind == SD_CMD ? -g->pending.amount : g->pending.amount); /* effect on life */
    foil(l, CX - HUB_R, CY - HUB_R, CX + HUB_R, CY + HUB_R, LV_RADIUS_CIRCLE);
    text(l, U.f_hub, T->foil_ink, LV_OPA_COVER, CX, CY - 8, d);
    if (g->pending.kind == SD_CMD)
        text(l, U.f_small, T->foil_ink, LV_OPA_COVER, CX, CY - 36, TR(CMD));
    if (g->pending.src != g->pending.player) { /* source seat, drawn pointer, target seat */
        snprintf(sub, sizeof(sub), TR(SEAT_FMT), g->pending.src + 1);
        text(l, U.f_small, T->foil_ink, LV_OPA_COVER, CX - 17, CY + 26, sub);
        snprintf(sub, sizeof(sub), TR(SEAT_FMT), g->pending.player + 1);
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
        snprintf(buf, n, "%s %d", TR(POISON), sd_counter(p, SD_POISON));
    else if (cmd)
        snprintf(buf, n, "%s %d", TR(CMD), cmd);
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
    snprintf(buf, sizeof(buf), TR(SEAT_PROFILE_FMT), 1, U.profile_name);
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
        snprintf(buf, sizeof(buf), "%s  %s", TR(MENU_UNDO), d);
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
            snprintf(buf, sizeof(buf), "%s  %s", seat(i), TR(DEFEATED));
        else if (cnt[0])
            snprintf(buf, sizeof(buf), "%s  %s", seat(i), cnt);
        else
            snprintf(buf, sizeof(buf), TR(SEAT_FMT), i + 1);
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
        snprintf(t, sizeof(t), TR(TURN_FMT), g->turn);
        text(l, U.f_label, T->ink, LV_OPA_COVER, CX, CY - 22, t);
        snprintf(t, sizeof(t), TR(SEAT_FMT), g->active + 1);
        text(l, U.f_hub, T->seat[g->active], LV_OPA_COVER, CX, CY + 9, t);
        return;
    }
    char d[12], sub[24];
    bool cmd = g->pending.kind == SD_CMD;
    fmt_signed(d, sizeof(d), cmd ? -g->pending.amount : g->pending.amount); /* effect on life */
    text(l, U.f_hub, T->ink, LV_OPA_COVER, CX, cmd ? CY - 3 : CY - 8, d);
    if (cmd) /* the only mark that tells commander damage from combat damage: kept clear of the ring */
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, CY - 31, TR(CMD));
    if (g->pending.src != g->pending.player) { /* source seat, drawn pointer, target seat: each in its own light */
        snprintf(sub, sizeof(sub), TR(SEAT_FMT), g->pending.src + 1);
        text(l, U.f_small, T->seat[g->pending.src], LV_OPA_COVER, CX - 17, CY + 26, sub);
        snprintf(sub, sizeof(sub), TR(SEAT_FMT), g->pending.player + 1);
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
    snprintf(buf, sizeof(buf), TR(SEAT_PROFILE_FMT), 1, U.profile_name);
    text(l, U.f_label, lost ? T->ink : T->seat[0], opa, CX, 62, buf);
    snprintf(buf, sizeof(buf), "%d", p->life);
    if (lost) {
        skull(l, CX, 158, 150, T->ink, opa);
        text(l, U.f_label, T->ink, opa, CX, 262, TR(DEFEATED));
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
        snprintf(buf, sizeof(buf), "%s  %s", TR(MENU_UNDO), d);
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
            snprintf(buf, sizeof(buf), "%s  %s", seat(i), TR(DEFEATED));
        else if (cnt[0])
            snprintf(buf, sizeof(buf), "%s  %s", seat(i), cnt);
        else
            snprintf(buf, sizeof(buf), TR(SEAT_FMT), i + 1);
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

/* Player sheet, commander columns: one per other seat. The tally of column seat `s` is the damage the sheet's player
 * received from s, or (SD_S_CMD_SHOW = source) dealt to s. */
static void sheet_cell(int s, uint8_t *player, uint8_t *src)
{
    *player = S.v[SD_S_CMD_SHOW] ? (uint8_t)s : U.sel;
    *src = S.v[SD_S_CMD_SHOW] ? U.sel : (uint8_t)s;
}
static int sheet_col_x(int k, int cols) { return CX + (int)((k - (cols - 1) / 2.f) * (T->light ? 78 : 84)); }

static uint32_t button(lv_layer_t *l, int x1, int y1, int x2, int y2, bool primary);

/* One column: the seat (a lit chip when the knob drives it) over its tally, pending delta included. */
static void sheet_col(lv_layer_t *l, int s, int x, int y)
{
    const sd_game_t *g = &U.g;
    uint8_t p, src;
    char buf[16];
    sheet_cell(s, &p, &src);
    int v = sd_game_view(g, p).cmd[src][0];
    uint32_t ink = T->seat[s];
    if (s == U.sheet_src)
        ink = button(l, x - 22, y - 13, x + 22, y + 13, true);
    text(l, U.f_small, ink, LV_OPA_COVER, x, y, seat(s));
    if (v + 5 >= g->rules.cmd_lethal) /* "/ 21" once within 5 of lethal */
        snprintf(buf, sizeof(buf), "%d / %d", v, g->rules.cmd_lethal);
    else
        snprintf(buf, sizeof(buf), "%d", v);
    text(l, U.f_row, T->ink, LV_OPA_COVER, x, y + (T->light ? 24 : 26), buf);
}

static int menu_row_y(int i) { return 70 + i * 44; }

/* Text of menu row `i` (the undo row previews what it would undo). */
static const char *menu_label(int i, char *buf, size_t n)
{
    char act[40];
    switch (i) {
    case MENU_UNDO:
        if (!U.g.log_len)
            return TR(NOTHING_TO_UNDO);
        {
            const sd_action_t *last = &U.g.log[U.g.log_len - 1];
            if (last->automatic && U.g.log_len > 1) /* undone together with the hit that caused it */
                last--;
            fmt_action(act, sizeof(act), last);
            snprintf(buf, n, "%s  %s", TR(MENU_UNDO), act);
        }
        return buf;
    case MENU_LOG: return TR(MENU_LOG);
    case MENU_SHEET:
        snprintf(buf, n, TR(MENU_SHEET_FMT), U.sel + 1);
        return buf;
    case MENU_NEW: return TR(MENU_NEW);
    case MENU_SETTINGS: return TR(SETTINGS);
    default: return TR(MENU_BACK);
    }
}

/* Row text, or the label size for a line too long for `w` (the undo row previews a whole action). */
static const lv_font_t *menu_font(const char *s, int w)
{
    return lv_text_get_width(s, (uint32_t)strlen(s), U.f_row, 0) > w ? U.f_label : U.f_row;
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
            fill(l, 36, y - 21, UI_W - 37, y + 20, T->ink, 0);
        text(l, menu_font(s, 272), f ? T->ground : T->ink, LV_OPA_COVER, CX, y, s);
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
    text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 48, TR(MENU_LOG));
    line(l, 120, 70, 240, 70, T->ink, LV_OPA_COVER);
    if (!U.g.log_len)
        text(l, U.f_row, T->ink, LV_OPA_COVER, CX, CY, TR(LOG_EMPTY));
    for (int k = 0; k < LOG_ROWS; k++) {
        int idx = U.g.log_len - 1 - U.log_scroll - k;
        if (idx < 0)
            break;
        const sd_action_t *a = &U.g.log[idx];
        int y = 104 + k * 44;
        fill(l, 70, y - 7, 84, y + 7, log_hue(a), 0);
        log_row(l, a, y);
    }
    text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 326, TR(TAP_BACK));
}

static void draw_sheet(lv_layer_t *l)
{
    const sd_game_t *g = &U.g;
    const sd_player_t *p = &g->p[U.sel];
    char buf[48];
    ground(l);
    fill(l, 0, 0, UI_W - 1, 132, T->seat[U.sel], 0);
    snprintf(buf, sizeof(buf), TR(SEAT_FMT), U.sel + 1);
    text(l, U.f_title, T->seat_ink[U.sel], LV_OPA_COVER, CX, 84, buf);
    text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 152, TR(POISON));
    if (g->rules.poison_lethal)
        snprintf(buf, sizeof(buf), "%d / %d", sd_counter(p, SD_POISON), g->rules.poison_lethal);
    else
        snprintf(buf, sizeof(buf), "%d", sd_counter(p, SD_POISON));
    text(l, U.f_hub, T->ink, LV_OPA_COVER, CX, 182, buf);
    if (g->n_players > 1 && g->rules.cmd_lethal) {
        text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 272, S.v[SD_S_CMD_SHOW] ? TR(CMD_OUT) : TR(CMD));
        int cols = g->n_players - 1, k = 0;
        for (int s = 0; s < g->n_players; s++)
            if (s != U.sel)
                sheet_col(l, s, sheet_col_x(k++, cols), 294);
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
        snprintf(buf, sizeof(buf), TR(WINS_FMT), w + 1);
    else
        snprintf(buf, sizeof(buf), "%s", TR(DEFEAT));
    text(l, U.f_title, ink, LV_OPA_COVER, CX, 112, buf);
    text(l, U.f_label, ink, LV_OPA_COVER, CX, 158, TR(GAME_OVER));
    outline(l, 70, 200, 290, 246, ink);
    text(l, U.f_row, ink, LV_OPA_COVER, CX, 223, TR(MENU_UNDO));
    fill(l, 70, 258, 290, 304, ink, 0);
    text(l, U.f_row, field, LV_OPA_COVER, CX, 281, TR(MENU_NEW));
}

/* ---------- luminous theme: menus ---------- */

static void draw_menu_light(lv_layer_t *l)
{
    char buf[64];
    ground(l);
    for (int i = 0; i < MENU_ROWS; i++) {
        int y = menu_row_y(i);
        int dy = abs(y - CY) + 21, hw = (int)sqrtf((float)(R * R - dy * dy)) - 8;
        const char *s = menu_label(i, buf, sizeof(buf));
        hw = hw > 144 ? 144 : hw;
        if (i == U.menu_focus) /* focus = a lit glass pill, kept inside the round dial */
            pill(l, CX - hw, y - 21, CX + hw, y + 20, true);
        text(l, menu_font(s, 2 * hw - 28), T->ink, i == U.menu_focus ? LV_OPA_COVER : LV_OPA_70, CX, y, s);
    }
}

static void draw_log_light(lv_layer_t *l)
{
    ground(l);
    text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 48, TR(MENU_LOG));
    line(l, 120, 70, 240, 70, T->ink, LV_OPA_40);
    if (!U.g.log_len)
        text(l, U.f_row, T->ink, LV_OPA_70, CX, CY, TR(LOG_EMPTY));
    for (int k = 0; k < LOG_ROWS; k++) {
        int idx = U.g.log_len - 1 - U.log_scroll - k;
        if (idx < 0)
            break;
        const sd_action_t *a = &U.g.log[idx];
        int y = 104 + k * 44;
        fx_halo(l, 77, y, 14, log_hue(a), LV_OPA_COVER); /* a point of the seat's light; the text names it */
        log_row(l, a, y);
    }
    text(l, U.f_small, T->ink, LV_OPA_70, CX, 326, TR(TAP_BACK));
}

static void draw_sheet_light(lv_layer_t *l)
{
    const sd_game_t *g = &U.g;
    const sd_player_t *p = &g->p[U.sel];
    char buf[48];
    ground(l);
    knob_ring(l);
    fx_halo(l, CX, 84, 96, T->seat[U.sel], LV_OPA_30);
    snprintf(buf, sizeof(buf), TR(SEAT_FMT), U.sel + 1);
    text(l, U.f_title, T->ink, LV_OPA_COVER, CX, 84, buf);
    text(l, U.f_label, T->seat[U.sel], LV_OPA_COVER, CX, 152, TR(POISON));
    if (g->rules.poison_lethal)
        snprintf(buf, sizeof(buf), "%d / %d", sd_counter(p, SD_POISON), g->rules.poison_lethal);
    else
        snprintf(buf, sizeof(buf), "%d", sd_counter(p, SD_POISON));
    text(l, U.f_hub, T->ink, LV_OPA_COVER, CX, 182, buf);
    if (g->n_players > 1 && g->rules.cmd_lethal) {
        text(l, U.f_label, T->seat[U.sel], LV_OPA_COVER, CX, 268, S.v[SD_S_CMD_SHOW] ? TR(CMD_OUT) : TR(CMD));
        int cols = g->n_players - 1, k = 0;
        for (int s = 0; s < g->n_players; s++)
            if (s != U.sel)
                sheet_col(l, s, sheet_col_x(k++, cols), 290);
        if (g->has_pending && g->pending.kind == SD_CMD) /* the same closing arc as for poison */
            arc(l, KNOB_R + 2, -90.f, -90.f + 360.f * pending_progress(), 5, T->ink, LV_OPA_COVER);
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
        snprintf(buf, sizeof(buf), TR(WINS_FMT), w + 1);
    } else
        snprintf(buf, sizeof(buf), "%s", TR(DEFEAT));
    text(l, U.f_title, T->ink, w >= 0 ? LV_OPA_COVER : LV_OPA_60, CX, 112, buf);
    text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 158, TR(GAME_OVER));
    pill(l, 70, 200, 290, 246, false);
    text(l, U.f_row, T->ink, LV_OPA_COVER, CX, 223, TR(MENU_UNDO));
    pill(l, 70, 258, 290, 304, true);
    text(l, U.f_row, T->ink, LV_OPA_COVER, CX, 281, TR(MENU_NEW));
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

/* Home, radial: START (new game) at the centre, REPRENDRE below when a game is saved, settings top left. The knob
 * moves the focus between these three, a tap on one (or anywhere on bare ground, for the focused one) runs it.
 * Update, host and join are shown dimmed and do nothing yet (the connectivity lots give them a destination). */
extern const lv_image_dsc_t ui_home_icons_a8[4];
#define NEW_Y 160
#define NEW_R 64
#define RESUME_X1 116
#define RESUME_Y1 276
#define RESUME_X2 244
#define RESUME_Y2 326
static void open_settings(bool from_game);

/* "New game" opens on the format the settings name. */
static void default_profile(void)
{
    U.profile = U.n_profiles ? S.v[SD_S_PROFILE] % U.n_profiles : 0;
    U.players = U.n_profiles ? U.profiles[U.profile].players : 1;
}

static void home_activate(int action)
{
    if (action == HOME_START && U.n_profiles) { /* opens on the last format played, at first the default one */
        U.new_step = NEW_FORMAT;
        U.scr = UI_SCR_NEW;
    } else if (action == HOME_RESUME && U.has_resume)
        start_game(&U.resume);
    else if (action == HOME_SETTINGS)
        open_settings(false);
}

static void draw_home(lv_layer_t *l)
{
    const struct { /* order = ui_home_icons_a8[] (ui/assets/icons2a8.py) */
        int x, y, r;
        const char *label;
    } off[] = {{92, 96, 24, NULL}, {268, 96, 24, NULL}, {76, 206, 34, TR(HOST)}, {284, 206, 34, TR(JOIN)}};
    char buf[24];
    ground(l);
    fx_image(l, &ui_wordmark_small_a8, CX, 46, T->ink, LV_OPA_COVER);
    for (size_t i = 0; i < sizeof(off) / sizeof(off[0]); i++) {
        bool lit = i == 0 && U.home_focus == HOME_SETTINGS;
        uint32_t ik = button(l, off[i].x - off[i].r, off[i].y - off[i].r, off[i].x + off[i].r, off[i].y + off[i].r, lit);
        fx_image(l, &ui_home_icons_a8[i], off[i].x, off[i].y, ik, i == 0 ? LV_OPA_COVER : LV_OPA_40);
        if (off[i].label)
            text(l, U.f_small, T->ink, LV_OPA_40, off[i].x, off[i].y + off[i].r + 16, off[i].label);
    }
    /* the focused action is the lit one; with a saved game that is REPRENDRE at first: a new game would replace it */
    if (T->light && U.home_focus == HOME_START)
        fx_ring(l, CX, NEW_Y, NEW_R, T->ink, LV_OPA_70);
    uint32_t ink = button(l, CX - NEW_R, NEW_Y - NEW_R, CX + NEW_R, NEW_Y + NEW_R, U.home_focus == HOME_START);
    fx_image(l, &ui_logo_small_a8, CX, NEW_Y - 12, ink, LV_OPA_COVER); /* the product's mark starts a game */
    text(l, U.f_row, ink, LV_OPA_COVER, CX, NEW_Y + 38, TR(START));
    if (U.has_resume) {
        ink = button(l, RESUME_X1, RESUME_Y1, RESUME_X2, RESUME_Y2, U.home_focus == HOME_RESUME);
        text(l, U.f_small, ink, LV_OPA_COVER, CX, RESUME_Y1 + 15, TR(RESUME));
        snprintf(buf, sizeof(buf), TR(RESUME_SUB_FMT), U.resume.turn, U.resume.n_players);
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

static bool new_last(void) { return U.new_step == NEW_VIEW || (U.new_step == NEW_PLAYERS && U.players == 1); }

static void draw_new(lv_layer_t *l)
{
    const sd_profile_t *p = &U.profiles[U.profile];
    char buf[64], name[24];
    ground(l);
    if (T->light)
        knob_ring(l);
    /* ponytail: one extra language, one extra field; a per-language map in the profile if a third one comes */
    snprintf(buf, sizeof(buf), "%s", ui_langs[S.v[SD_S_LANG] % UI_LANG_COUNT] == &ui_lang_en && p->family_en[0] ? p->family_en : p->family);
    upper(buf);
    bool solo_view = S.v[SD_S_VIEW] == SD_VIEW_SOLO;
    text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 60,
         U.new_step == NEW_VIEW ? TR(GAME_SCREEN) : U.new_step == NEW_PLAYERS ? TR(PLAYERS) : buf[0] ? buf : TR(FORMAT));
    /* the format name in the product name's face and case (Original Motion runs wide: 48 px past 300 px) */
    snprintf(name, sizeof(name), "%s", p->name);
    const lv_font_t *nf = U.f_name[lv_text_get_width(name, strlen(name), U.f_name[0], 0) > 300];
    text(l, nf, T->ink, LV_OPA_COVER, CX, 112, name);
    if (U.new_step == NEW_VIEW) { /* what the dial shows during the game: every seat, or my life alone */
        text(l, U.f_hub, T->ink, LV_OPA_COVER, CX, 170, solo_view ? TR(SOLO) : TR(ARENA));
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 212, solo_view ? TR(VIEW_SOLO_SUB) : TR(VIEW_ARENA_SUB));
        for (int i = 0; i < 2; i++) {
            fill(l, CX - 10 + i * 14, 233, CX - 4 + i * 14, 239, T->ink, T->light ? LV_RADIUS_CIRCLE : 0);
            if (i != solo_view)
                fill(l, CX - 9 + i * 14, 234, CX - 5 + i * 14, 238, T->ground, T->light ? LV_RADIUS_CIRCLE : 0);
        }
        goto buttons;
    }
    /* only the rules the format has: no commander damage, no mention of it */
    snprintf(buf, sizeof(buf), TR(LIFE_FMT), p->rules.start_life);
    if (p->rules.cmd_lethal)
        appendf(buf, sizeof(buf), DOT "%s %d", TR(CMD), p->rules.cmd_lethal);
    if (p->rules.poison_lethal)
        appendf(buf, sizeof(buf), DOT "%s %d", TR(POISON), p->rules.poison_lethal);
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
            snprintf(buf, sizeof(buf), TR(SEAT_FMT), i + 1);
            text(l, U.f_small, T->light ? T->ink : on ? T->seat_ink[i] : T->seat[i],
                 on || !T->light ? LV_OPA_COVER : LV_OPA_40, x, y, buf);
        }
        if (U.players == 1)
            snprintf(buf, sizeof(buf), "%s", TR(SOLO));
        else
            snprintf(buf, sizeof(buf), TR(PLAYERS_FMT), U.players);
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 240, buf);
    }
buttons:
    for (int k = -1; k <= 1; k += 2) /* chevrons: a tap on either side steps like the knob */
        text(l, U.f_row, T->ink, LV_OPA_60, CX + k * 124, 206, k < 0 ? "\xE2\x80\xB9" : "\xE2\x80\xBA");
    button(l, 66, NEW_BTN_Y1 + 2, 110, NEW_BTN_Y2 - 2, false); /* back */
    fx_icon(l, FX_ICON_UNDO, 88, (NEW_BTN_Y1 + NEW_BTN_Y2) / 2, T->ink, LV_OPA_COVER);
    uint32_t ink = button(l, 124, NEW_BTN_Y1, 264, NEW_BTN_Y2, true);
    text(l, U.f_row, ink, LV_OPA_COVER, 194, (NEW_BTN_Y1 + NEW_BTN_Y2) / 2, new_last() ? TR(PLAY) : TR(NEXT));
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
    snprintf(buf, sizeof(buf), TR(SEAT_FMT), U.sel + 1);
    text(l, U.f_label, T->seat[U.sel], LV_OPA_COVER, CX, 104, buf);
    text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 134, MARKER_NAME[U.marker]);
    const char *v = buf;
    if (kind == SD_POISON && g->rules.poison_lethal)
        snprintf(buf, sizeof(buf), "%d / %d", sd_counter(p, SD_POISON), g->rules.poison_lethal);
    else if (kind < SD_ASCEND)
        snprintf(buf, sizeof(buf), "%d", sd_counter(p, kind));
    else if (kind == SD_ASCEND)
        v = p->ascended ? TR(YES) : TR(NO);
    else if (kind == SD_DAYNIGHT)
        v = g->daynight == SD_DN_DAY ? TR(DAY) : g->daynight == SD_DN_NIGHT ? TR(NIGHT) : NONE;
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
        text(l, U.f_small, T->ink, LV_OPA_70, CX, 232, TR(MENU_BACK));
}

/* ---------- settings: the sections grid, the list of one section, text / PIN entry ---------- */

enum { SEC_GAME, SEC_PLAYERS, SEC_TURNS, SEC_DISPLAY, SEC_NETWORK, SEC_DEVICE, SEC_N };
#define SET_BACK SEC_N /* the grid's seventh focus stop: its back button */

typedef enum {
    ROW_BOOL,    /* yes / no */
    ROW_ENUM,    /* named values: the strings `values`, `values + 1`... */
    ROW_LANG,    /* a language of ui_langs[] */
    ROW_PROFILE, /* a shipped format */
    ROW_INT,     /* a number */
    ROW_AUTO,    /* a number, 0 = whatever the format says */
    ROW_MS,      /* a duration */
    ROW_INFO,    /* nothing to change */
    ROW_ACTION,  /* runs on a tap; `confirm` = on the second tap */
    ROW_BACK
} row_kind_t;
enum { INFO_GAME_TYPE, INFO_VERSION };
enum { ACT_RESET_GAME, ACT_NAME, ACT_NAME_RANDOM, ACT_SETUP, ACT_PIN, ACT_MSG, ACT_FACTORY };

typedef struct {
    uint16_t label; /* ui_str_t */
    uint8_t kind;   /* row_kind_t */
    uint8_t id;     /* the setting (sd_setting_id_t), INFO_* or ACT_* */
    uint8_t step;
    uint16_t values;
    bool confirm;
} row_t;

static const row_t ROWS_GAME[] = {
    {STR_R_GAME_TYPE, ROW_INFO, INFO_GAME_TYPE, 0, 0, false},
    {STR_R_DEFAULT_FORMAT, ROW_PROFILE, SD_S_PROFILE, 1, 0, false},
    {STR_GAME_SCREEN, ROW_ENUM, SD_S_VIEW, 1, STR_ARENA, false},
    {STR_R_TURN_ORDER, ROW_ENUM, SD_S_TURN_ORDER, 1, STR_V_CLOCKWISE, false},
    {STR_R_START_LIFE, ROW_AUTO, SD_S_START_LIFE, 5, 0, false},
    {STR_R_CLICKS, ROW_INT, SD_S_CLICKS, 1, 0, false},
    {STR_R_START_ON, ROW_ENUM, SD_S_START_ON, 1, STR_V_HOME, false},
    {STR_R_ROTATE, ROW_BOOL, SD_S_ROTATE_SEATS, 1, 0, false},
    {STR_R_AUTO_KO, ROW_BOOL, SD_S_AUTO_KO, 1, 0, false},
    {STR_R_AUTO_WIN, ROW_BOOL, SD_S_AUTO_WIN, 1, 0, false},
    {STR_R_CMD_LIFE, ROW_BOOL, SD_S_CMD_LIFE, 1, 0, false},
    {STR_R_DRAG_CMD, ROW_BOOL, SD_S_DRAG_CMD, 1, 0, false},
    {STR_R_CMD_LETHAL, ROW_AUTO, SD_S_CMD_LETHAL, 1, 0, false},
    {STR_R_POISON_LETHAL, ROW_AUTO, SD_S_POISON_LETHAL, 1, 0, false},
    {STR_R_CMD_SHOW, ROW_ENUM, SD_S_CMD_SHOW, 1, STR_V_TARGET, false},
    {STR_R_COMMIT, ROW_MS, SD_S_COMMIT_MS, 100, 0, false},
    {STR_R_RESET_GAME, ROW_ACTION, ACT_RESET_GAME, 0, 0, true},
    {STR_MENU_BACK, ROW_BACK, 0, 0, 0, false},
};

static const row_t ROWS_DEVICE[] = {
    {STR_R_VERSION, ROW_INFO, INFO_VERSION, 0, 0, false},
    {STR_R_LANG, ROW_LANG, SD_S_LANG, 1, 0, false},
    {STR_R_NAME, ROW_ACTION, ACT_NAME, 0, 0, false},
    {STR_R_NAME_RANDOM, ROW_ACTION, ACT_NAME_RANDOM, 0, 0, false},
    {STR_R_SETUP, ROW_ACTION, ACT_SETUP, 0, 0, false},
    {STR_PIN, ROW_ACTION, ACT_PIN, 0, 0, false}, /* asks twice to remove a PIN, see row_confirm() */
    {STR_R_PIN_BOOT, ROW_BOOL, SD_S_PIN_BOOT, 1, 0, false},
    {STR_OWNER_MSG, ROW_ACTION, ACT_MSG, 0, 0, false},
    {STR_R_KNOB_DIR, ROW_ENUM, SD_S_KNOB_REVERSE, 1, STR_V_NORMAL, false},
    {STR_R_KNOB_COUNTS, ROW_INT, SD_S_KNOB_COUNTS, 1, 0, false},
    {STR_R_HAPTIC, ROW_ENUM, SD_S_HAPTIC, 1, STR_V_HAPTIC_OFF, false},
    {STR_R_FACTORY, ROW_ACTION, ACT_FACTORY, 0, 0, true},
    {STR_MENU_BACK, ROW_BACK, 0, 0, 0, false},
};

#define NROWS(a) (int)(sizeof(a) / sizeof((a)[0]))
static const struct {
    uint16_t name;
    const row_t *rows; /* NULL = not there yet: shown dimmed, "BIENTÔT" */
    int n;
} SECTIONS[SEC_N] = {
    [SEC_GAME] = {STR_SEC_GAME, ROWS_GAME, NROWS(ROWS_GAME)}, [SEC_PLAYERS] = {STR_SEC_PLAYERS, NULL, 0},
    [SEC_TURNS] = {STR_SEC_TURNS, NULL, 0},                  [SEC_DISPLAY] = {STR_SEC_DISPLAY, NULL, 0},
    [SEC_NETWORK] = {STR_SEC_NETWORK, NULL, 0},              [SEC_DEVICE] = {STR_SEC_DEVICE, ROWS_DEVICE, NROWS(ROWS_DEVICE)},
};

static void unlocked(void);
static void boot_done(void);

/* text / PIN entry: the knob picks a character, a tap on it appends it */
enum { ED_GATE, ED_BOOT, ED_PIN_NEW, ED_NAME, ED_MSG }; /* the first three take a PIN */
static const char ALPHA[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-'!?"; /* ponytail: no accents, add them when asked */
static const char DIGITS[] = "0123456789";
#define ED_Y 168
#define ED_R 40

static bool ed_pin(void) { return U.ed.purpose <= ED_PIN_NEW; }
static const char *ed_alpha(void) { return ed_pin() ? DIGITS : ALPHA; }

static void edit_open(int purpose, const char *init, int max)
{
    memset(&U.ed, 0, sizeof(U.ed));
    U.ed.purpose = (uint8_t)purpose;
    U.ed.max = (uint8_t)max;
    U.ed.back = U.scr;
    snprintf(U.ed.buf, (size_t)max + 1, "%s", init ? init : "");
    U.ed.len = (uint8_t)strlen(U.ed.buf);
    U.ed.ch = ed_pin() ? 0 : 1;
    U.scr = UI_SCR_EDIT;
}

static void open_settings(bool from_game)
{
    U.set_from_game = from_game;
    U.set_focus = SEC_GAME;
    if (S.pin[0])
        edit_open(ED_GATE, NULL, 4); /* comes back to where it was opened from if given up */
    else
        U.scr = UI_SCR_SETTINGS;
}

static void edit_done(void)
{
    switch (U.ed.purpose) {
    case ED_GATE:
    case ED_BOOT:
        /* ponytail: no limit on attempts; the PIN keeps the table honest, it does not resist an attacker */
        if (strcmp(U.ed.buf, S.pin)) {
            U.ed.wrong = true;
            U.ed.len = 0;
            U.ed.buf[0] = '\0';
            haptic(UI_HAPTIC_DEFEAT);
        } else if (U.ed.purpose == ED_BOOT)
            unlocked();
        else
            U.scr = UI_SCR_SETTINGS;
        return;
    case ED_PIN_NEW: memcpy(S.pin, U.ed.buf, sizeof(S.pin)); break;
    default: {
        while (U.ed.len && U.ed.buf[U.ed.len - 1] == ' ') /* no trailing blanks */
            U.ed.buf[--U.ed.len] = '\0';
        char *dst = U.ed.purpose == ED_NAME ? S.name : S.owner_msg;
        snprintf(dst, (size_t)U.ed.max + 1, "%s", U.ed.buf);
    }
    }
    U.dirty = true;
    settings_save();
    U.scr = U.ed.back;
}

/* ponytail: a generator stirred by the time of the taps; plenty for a name, not for dice (plan #7 brings the HAL's) */
static uint32_t rnd(void)
{
    U.seed = U.seed * 1664525u + 1013904223u + U.now;
    return U.seed >> 10;
}

/* Two words in the language in use: "DRAGON ÉCARLATE", "CRIMSON DRAGON". */
static void random_name(void)
{
    const ui_lang_t *L = ui_langs[S.v[SD_S_LANG] % UI_LANG_COUNT];
    const char *n = L->noun[rnd() % (uint32_t)L->n_noun], *a = L->adj[rnd() % (uint32_t)L->n_adj];
    snprintf(S.name, sizeof(S.name), "%s %s", L->adj_first ? a : n, L->adj_first ? n : a);
    U.dirty = true;
}

/* Everything back as shipped: no game, no settings. */
static void factory_reset(void)
{
    if (U.hooks.factory_reset)
        U.hooks.factory_reset();
    sd_settings_defaults(&S);
    U.has_resume = U.in_game = U.dirty = false;
    default_profile();
    if (U.hooks.save_settings) /* the HAL takes the defaults back too (knob, haptics) */
        U.hooks.save_settings(&S);
    boot_done(); /* as out of the box: the assistant */
}

static void settings_sec_xy(int i, int *x, int *y)
{
    *x = i % 2 ? 252 : 108;
    *y = 118 + (i / 2) * 54;
}

static void draw_settings(lv_layer_t *l)
{
    ground(l);
    if (T->light)
        knob_ring(l);
    text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 56, TR(SETTINGS));
    for (int i = 0; i < SEC_N; i++) {
        int x, y;
        settings_sec_xy(i, &x, &y);
        uint32_t ink = button(l, x - 66, y - 22, x + 66, y + 22, i == U.set_focus);
        text(l, U.f_small, ink, SECTIONS[i].rows ? LV_OPA_COVER : LV_OPA_40, x, y, tr(SECTIONS[i].name));
    }
    if (U.set_focus < SEC_N && !SECTIONS[U.set_focus].rows)
        text(l, U.f_small, T->ink, LV_OPA_70, CX, 264, TR(SOON));
    button(l, CX - 22, 282, CX + 22, 326, U.set_focus == SET_BACK);
    fx_icon(l, FX_ICON_UNDO, CX, 304, U.set_focus == SET_BACK && !T->light ? T->ground : T->ink, LV_OPA_COVER);
}

static void settings_activate(int i)
{
    if (i == SET_BACK) {
        settings_save();
        /* a rule changed under the game may have ended it */
        U.scr = !U.set_from_game ? UI_SCR_HOME : sd_game_over(&U.g) ? UI_SCR_OVER : UI_SCR_MENU;
    } else if (SECTIONS[i].rows) {
        U.section = i;
        U.row = 0;
        U.editing = false;
        U.armed = -1;
        U.scr = UI_SCR_LIST;
    }
}

static const row_t *cur_row(void) { return &SECTIONS[U.section].rows[U.row]; }

/* What the row shows next to its label; empty for a plain action. */
static void row_value(const row_t *r, int idx, char *buf, size_t n)
{
    int v = r->kind <= ROW_MS ? S.v[r->id] : 0;
    buf[0] = '\0';
    switch (r->kind) {
    case ROW_BOOL: snprintf(buf, n, "%s", v ? TR(YES) : TR(NO)); break;
    case ROW_ENUM: snprintf(buf, n, "%s", tr(r->values + v)); break;
    case ROW_LANG: snprintf(buf, n, "%s", ui_langs[v % UI_LANG_COUNT]->name); break;
    case ROW_PROFILE:
        snprintf(buf, n, "%s", U.n_profiles ? U.profiles[v % U.n_profiles].name : NONE);
        upper(buf);
        break;
    case ROW_INT: snprintf(buf, n, "%d", v); break;
    case ROW_AUTO:
        if (v)
            snprintf(buf, n, "%d", v);
        else
            snprintf(buf, n, "%s", TR(FROM_FORMAT));
        break;
    case ROW_MS: snprintf(buf, n, "%d MS", v); break;
    case ROW_INFO:
        snprintf(buf, n, "%s", r->id == INFO_GAME_TYPE ? TR(MAGIC) : U.hooks.version ? U.hooks.version : "DEV");
        break;
    case ROW_ACTION:
        if (idx == U.armed)
            snprintf(buf, n, "%s", TR(CONFIRM));
        else if (r->id == ACT_NAME || r->id == ACT_MSG) {
            const char *v = r->id == ACT_NAME ? S.name : S.owner_msg;
            snprintf(buf, n, "%s", v[0] ? v : NONE);
        } else if (r->id == ACT_PIN)
            snprintf(buf, n, "%s", S.pin[0] ? "\xE2\x80\xA2\xE2\x80\xA2\xE2\x80\xA2\xE2\x80\xA2" : TR(PIN_NONE));
        break;
    default: break;
    }
}

/* Knob step on the row being edited. Named values go round, numbers stop at their bounds. */
static void row_change(const row_t *r, int d)
{
    int v = S.v[r->id], lo = sd_setting_min(r->id);
    int n = r->kind == ROW_LANG ? UI_LANG_COUNT : r->kind == ROW_PROFILE ? U.n_profiles : sd_setting_max(r->id) - lo + 1;
    if (r->kind <= ROW_PROFILE)
        v = n ? lo + ((v - lo + d) % n + n) % n : v;
    else
        v += d * r->step;
    sd_setting_set(&S, r->id, v);
    U.dirty = true;
    if (r->id == SD_S_PROFILE)
        default_profile();
}

static void setup_open(bool first);

static void row_action(const row_t *r)
{
    switch (r->id) {
    case ACT_RESET_GAME: restart_game(); break;
    case ACT_NAME: edit_open(ED_NAME, S.name, sizeof(S.name) - 1); break;
    case ACT_NAME_RANDOM:
        random_name();
        settings_save();
        break;
    case ACT_PIN:
        if (S.pin[0]) { /* confirmed: no PIN any more */
            S.pin[0] = '\0';
            U.dirty = true;
            settings_save();
        } else
            edit_open(ED_PIN_NEW, NULL, 4);
        break;
    case ACT_MSG: edit_open(ED_MSG, S.owner_msg, sizeof(S.owner_msg) - 1); break;
    case ACT_SETUP: setup_open(false); break;
    case ACT_FACTORY: factory_reset(); break;
    }
}

/* Does the action want a second tap? What cannot be taken back does. */
static bool row_confirm(const row_t *r) { return r->confirm || (r->id == ACT_PIN && S.pin[0]); }

/* The focused row sits in the middle, large; two rows above and below show what is around it. */
#define LIST_Y1 142
#define LIST_Y2 218
static void draw_list(lv_layer_t *l)
{
    static const int ys[] = {84, 116, 0, 244, 276};
    const row_t *rows = SECTIONS[U.section].rows;
    char buf[80], val[40];
    ground(l);
    if (T->light)
        knob_ring(l);
    text(l, U.f_small, T->ink, LV_OPA_70, CX, 46, tr(SECTIONS[U.section].name));
    for (int k = -2; k <= 2; k++) {
        int i = U.row + k;
        if (!k || i < 0 || i >= SECTIONS[U.section].n)
            continue;
        row_value(&rows[i], i, val, sizeof(val));
        snprintf(buf, sizeof(buf), "%s%s%s", tr(rows[i].label), val[0] ? "  " : "", val);
        text(l, U.f_small, T->ink, abs(k) == 1 ? LV_OPA_80 : LV_OPA_60, CX, ys[k + 2], buf); /* 60 % keeps 4.5:1 */
    }
    const row_t *r = cur_row();
    uint32_t ink = button(l, 30, LIST_Y1, UI_W - 31, LIST_Y2, U.editing || U.armed == U.row);
    row_value(r, U.row, val, sizeof(val));
    if (val[0]) {
        text(l, U.f_small, ink, LV_OPA_COVER, CX, 162, tr(r->label));
        text(l, U.f_row, ink, LV_OPA_COVER, CX, 196, val);
    } else
        text(l, U.f_row, ink, LV_OPA_COVER, CX, 180, tr(r->label));
    if (U.editing)
        for (int k = -1; k <= 1; k += 2) /* a tap on either side steps like the knob */
            text(l, U.f_row, ink, LV_OPA_COVER, CX + k * 128, 194, k < 0 ? "\xE2\x80\xB9" : "\xE2\x80\xBA");
}

static void list_tap(int x, int y)
{
    int k = y < 100 ? -2 : y < LIST_Y1 - 6 ? -1 : y <= LIST_Y2 + 6 ? 0 : y < 262 ? 1 : 2;
    const row_t *r = cur_row();
    if (k) { /* another row: it takes the focus */
        int i = U.row + k;
        if (i < 0 || i >= SECTIONS[U.section].n)
            return;
        U.row = i;
        U.editing = false;
        U.armed = -1;
        settings_save();
    } else if (r->kind == ROW_BACK) {
        settings_save();
        U.scr = UI_SCR_SETTINGS;
    } else if (r->kind == ROW_ACTION) {
        if (row_confirm(r) && U.armed != U.row)
            U.armed = U.row;
        else {
            U.armed = -1;
            row_action(r);
        }
    } else if (r->kind != ROW_INFO) {
        if (U.editing && (x < 76 || x >= UI_W - 76))
            row_change(r, x < CX ? -1 : 1);
        else {
            U.editing = !U.editing; /* tap = enter, tap = leave */
            if (!U.editing)
                settings_save();
        }
    }
}

static void draw_edit(lv_layer_t *l)
{
    static const uint16_t TITLE[] = {STR_PIN, STR_PIN, STR_PIN_NEW, STR_DEVICE_NAME, STR_OWNER_MSG};
    const int mid = (NEW_BTN_Y1 + NEW_BTN_Y2) / 2;
    char c[2] = {ed_alpha()[U.ed.ch], '\0'};
    ground(l);
    if (T->light)
        knob_ring(l);
    /* at power-on the owner's message is the title: whoever finds the device reads it */
    text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 56,
         U.ed.purpose == ED_BOOT && S.owner_msg[0] ? S.owner_msg : tr(TITLE[U.ed.purpose]));
    if (ed_pin())
        for (int i = 0; i < 4; i++) { /* four slots, filled as the digits come */
            int x = CX - 45 + i * 30;
            fill(l, x - 7, 97, x + 7, 111, T->ink, T->light ? LV_RADIUS_CIRCLE : 0);
            if (i >= U.ed.len)
                fill(l, x - 5, 99, x + 5, 109, T->ground, T->light ? LV_RADIUS_CIRCLE : 0);
        }
    else
        text(l, U.f_label, T->ink, LV_OPA_COVER, CX, 104, U.ed.buf);
    if (T->light)
        fx_ring(l, CX, ED_Y, ED_R, T->ink, LV_OPA_70);
    uint32_t ink = button(l, CX - ED_R, ED_Y - ED_R, CX + ED_R, ED_Y + ED_R, true);
    if (c[0] == ' ')
        text(l, U.f_small, ink, LV_OPA_COVER, CX, ED_Y, TR(SPACE));
    else
        text(l, U.f_title, ink, LV_OPA_COVER, CX, ED_Y, c);
    for (int k = -1; k <= 1; k += 2)
        text(l, U.f_row, T->ink, LV_OPA_60, CX + k * 124, ED_Y, k < 0 ? "\xE2\x80\xB9" : "\xE2\x80\xBA");
    if (U.ed.wrong)
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 232, TR(PIN_WRONG));
    if (U.ed.purpose != ED_BOOT || U.ed.len) { /* erase, or give up when there is nothing to erase */
        button(l, 66, NEW_BTN_Y1 + 2, 110, NEW_BTN_Y2 - 2, false);
        fx_icon(l, FX_ICON_UNDO, 88, mid, T->ink, LV_OPA_COVER);
    }
    if (!ed_pin()) { /* a PIN is complete at four digits; a text is when its owner says so */
        ink = button(l, 124, NEW_BTN_Y1, 264, NEW_BTN_Y2, true);
        text(l, U.f_row, ink, LV_OPA_COVER, 194, mid, TR(OK));
    }
}

static void edit_knob(int d)
{
    int n = (int)strlen(ed_alpha());
    U.ed.ch = (uint8_t)(((U.ed.ch + d) % n + n) % n);
    haptic(UI_HAPTIC_DETENT);
}

static void edit_tap(int x, int y)
{
    if (y >= NEW_BTN_Y1 - 8 && y <= NEW_BTN_Y2 + 8) {
        if (x >= 58 && x < 118) {
            if (U.ed.len) { /* one character back, whole even when it takes several bytes */
                do
                    U.ed.len--;
                while (U.ed.len && ((uint8_t)U.ed.buf[U.ed.len] & 0xC0) == 0x80);
                U.ed.buf[U.ed.len] = '\0';
            } else if (U.ed.purpose != ED_BOOT)
                U.scr = U.ed.back;
        } else if (x >= 124 && x <= 264 && !ed_pin())
            edit_done();
    } else if ((x - CX) * (x - CX) + (y - ED_Y) * (y - ED_Y) <= 54 * 54) {
        if (U.ed.len < U.ed.max) {
            U.ed.buf[U.ed.len++] = ed_alpha()[U.ed.ch];
            U.ed.buf[U.ed.len] = '\0';
            U.ed.wrong = false;
            haptic(UI_HAPTIC_COMMIT);
        }
        if (ed_pin() && U.ed.len == 4)
            edit_done();
    } else if (y < 240 && (x < 110 || x >= 250))
        edit_knob(x < 110 ? -1 : 1);
}

/* ---------- first-start assistant: language, welcome, default format, device name, then three steps one may pass
 * (knob calibration, touch test, gestures). Run again from the settings (APPAREIL > ASSISTANT). ---------- */

enum { SU_LANG, SU_WELCOME, SU_FORMAT, SU_NAME, SU_KNOB, SU_TOUCH, SU_GESTURES };
#define SU_TARGET_R 128 /* touch targets: top, right, bottom, left, at this distance from the centre */
#define SU_NAME_Y 188

static void setup_open(bool first)
{
    U.setup_first = first;
    U.setup_step = SU_LANG;
    U.scr = UI_SCR_SETUP;
}

static void setup_target(int i, int *x, int *y)
{
    static const int dx[] = {0, 1, 0, -1}, dy[] = {-1, 0, 1, 0};
    *x = CX + dx[i] * SU_TARGET_R;
    *y = CY + dy[i] * SU_TARGET_R;
}

static void setup_next(void)
{
    switch (U.setup_step) {
    case SU_KNOB:
        /* The owner turned 10 clicks to the right. Fewer or more detents came in: the counts per detent are off by
         * that ratio; they came in negative: the knob is wired the other way. Nothing turned = step passed. */
        if (U.cal) {
            if (U.cal < 0)
                sd_setting_set(&S, SD_S_KNOB_REVERSE, !S.v[SD_S_KNOB_REVERSE]);
            sd_setting_set(&S, SD_S_KNOB_COUNTS, (abs(U.cal) * S.v[SD_S_KNOB_COUNTS] + 5) / 10);
            U.dirty = true;
            settings_save(); /* the HAL applies it at once: the rest of the assistant already feels right */
        }
        break;
    case SU_GESTURES:
        sd_setting_set(&S, SD_S_SETUP_DONE, 1);
        U.dirty = true;
        settings_save();
        if (U.setup_first)
            unlocked();
        else
            U.scr = UI_SCR_LIST;
        return;
    }
    U.setup_step++;
    U.cal = 0;
    U.touched = 0;
    if (U.setup_step == SU_NAME && !S.name[0])
        random_name();
}

static void setup_knob(int d)
{
    switch (U.setup_step) {
    case SU_LANG:
        sd_setting_set(&S, SD_S_LANG, ((S.v[SD_S_LANG] + d) % UI_LANG_COUNT + UI_LANG_COUNT) % UI_LANG_COUNT);
        break;
    case SU_FORMAT:
        if (!U.n_profiles)
            return;
        sd_setting_set(&S, SD_S_PROFILE, ((S.v[SD_S_PROFILE] + d) % U.n_profiles + U.n_profiles) % U.n_profiles);
        default_profile();
        break;
    case SU_NAME: random_name(); break;
    case SU_KNOB: U.cal += d; break;
    default: return;
    }
    U.dirty = true;
    haptic(UI_HAPTIC_DETENT);
}

/* where we are in a short list: one dot per entry, the current one full */
static void dots(lv_layer_t *l, int n, int cur)
{
    for (int i = 0; i < n; i++) {
        int x = CX - (n - 1) * 7 + i * 14;
        fill(l, x - 3, 207, x + 3, 213, T->ink, T->light ? LV_RADIUS_CIRCLE : 0);
        if (i != cur)
            fill(l, x - 2, 208, x + 2, 212, T->ground, T->light ? LV_RADIUS_CIRCLE : 0);
    }
}

static void draw_setup(lv_layer_t *l)
{
    const int mid = (NEW_BTN_Y1 + NEW_BTN_Y2) / 2;
    const char *go = TR(NEXT);
    char buf[64];
    bool chevrons = false;
    uint32_t ink;
    ground(l);
    if (T->light)
        knob_ring(l);
    switch (U.setup_step) {
    case SU_LANG: /* the one screen that cannot know its language: the title says it in each */
        buf[0] = '\0';
        for (int i = 0; i < UI_LANG_COUNT; i++)
            appendf(buf, sizeof(buf), "%s%s", i ? " / " : "", ui_langs[i]->str[STR_R_LANG]);
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 60, buf);
        text(l, U.f_hub, T->ink, LV_OPA_COVER, CX, 136, ui_langs[S.v[SD_S_LANG] % UI_LANG_COUNT]->name);
        dots(l, UI_LANG_COUNT, S.v[SD_S_LANG] % UI_LANG_COUNT);
        chevrons = true;
        break;
    case SU_WELCOME:
        fx_image(l, &ui_wordmark_small_a8, CX, 52, T->ink, LV_OPA_COVER);
        fx_image(l, &ui_logo_small_a8, CX, 128, T->ink, LV_OPA_COVER);
        text(l, U.f_hub, T->ink, LV_OPA_COVER, CX, 208, TR(WELCOME));
        go = TR(SETUP_START);
        break;
    case SU_FORMAT: {
        snprintf(buf, sizeof(buf), "%s" DOT "%s", TR(MAGIC), TR(R_DEFAULT_FORMAT));
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 60, buf);
        if (!U.n_profiles)
            break;
        const sd_profile_t *p = &U.profiles[U.profile];
        const lv_font_t *nf = U.f_name[lv_text_get_width(p->name, strlen(p->name), U.f_name[0], 0) > 300];
        text(l, nf, T->ink, LV_OPA_COVER, CX, 112, p->name);
        snprintf(buf, sizeof(buf), TR(LIFE_FMT), p->rules.start_life);
        if (p->rules.cmd_lethal)
            appendf(buf, sizeof(buf), DOT "%s %d", TR(CMD), p->rules.cmd_lethal);
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 154, buf);
        dots(l, U.n_profiles, U.profile);
        chevrons = true;
        break;
    }
    case SU_NAME:
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 60, TR(DEVICE_NAME));
        text(l, U.f_row, T->ink, LV_OPA_COVER, CX, 116, S.name);
        ink = button(l, 62, SU_NAME_Y - 22, 174, SU_NAME_Y + 22, false);
        text(l, U.f_small, ink, LV_OPA_COVER, 118, SU_NAME_Y, TR(RANDOM));
        ink = button(l, 186, SU_NAME_Y - 22, 298, SU_NAME_Y + 22, false);
        text(l, U.f_small, ink, LV_OPA_COVER, 242, SU_NAME_Y, TR(EDIT));
        break;
    case SU_KNOB:
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 60, TR(SETUP_KNOB));
        text(l, U.f_small, T->ink, LV_OPA_70, CX, 100, TR(SETUP_KNOB_1));
        text(l, U.f_small, T->ink, LV_OPA_70, CX, 124, TR(SETUP_KNOB_2));
        snprintf(buf, sizeof(buf), "%d", abs(U.cal));
        text(l, U.f_title, T->ink, LV_OPA_COVER, CX, 190, buf);
        if (!U.cal)
            go = TR(SKIP);
        break;
    case SU_TOUCH: /* a test card: rings, the four seat colours, and four targets to hit */
        arc(l, 96, 0.f, 360.f, 1, T->ink, LV_OPA_40);
        arc(l, 150, 0.f, 360.f, 1, T->ink, LV_OPA_40);
        for (int i = 0; i < 4; i++) {
            int x, y;
            float a = (45.f + i * 90.f) * (float)M_PI / 180.f;
            x = CX + (int)(SU_TARGET_R * cosf(a));
            y = CY + (int)(SU_TARGET_R * sinf(a));
            fill(l, x - 12, y - 12, x + 12, y + 12, T->seat[i], T->light ? LV_RADIUS_CIRCLE : 0);
            setup_target(i, &x, &y);
            bool hit = U.touched >> i & 1;
            if (hit && T->light)
                fx_halo(l, x, y, 44, T->ink, LV_OPA_50);
            ink = button(l, x - 20, y - 20, x + 20, y + 20, hit);
            line(l, x - 8, y, x + 8, y, ink, LV_OPA_COVER);
            line(l, x, y - 8, x, y + 8, ink, LV_OPA_COVER);
        }
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 136, TR(SETUP_TOUCH));
        ink = button(l, CX - 66, 166, CX + 66, 214, true);
        text(l, U.f_row, ink, LV_OPA_COVER, CX, 190, U.touched == 15 ? TR(NEXT) : TR(SKIP));
        return; /* its button is in the middle: the bottom belongs to a target */
    case SU_GESTURES:
        text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 60, TR(SETUP_GESTURES));
        for (int i = 0; i < 4; i++)
            text(l, U.f_small, T->ink, LV_OPA_COVER, CX, 104 + i * 34, tr(STR_GESTURE_1 + i));
        go = TR(FINISH);
        break;
    }
    if (chevrons)
        for (int k = -1; k <= 1; k += 2)
            text(l, U.f_row, T->ink, LV_OPA_60, CX + k * 124, 206, k < 0 ? "\xE2\x80\xB9" : "\xE2\x80\xBA");
    if (U.setup_step || !U.setup_first) { /* back; at power-on the first step has nowhere to go back to */
        button(l, 66, NEW_BTN_Y1 + 2, 110, NEW_BTN_Y2 - 2, false);
        fx_icon(l, FX_ICON_UNDO, 88, mid, T->ink, LV_OPA_COVER);
    }
    ink = button(l, 124, NEW_BTN_Y1, 264, NEW_BTN_Y2, true);
    text(l, U.f_row, ink, LV_OPA_COVER, 194, mid, go);
}

static void setup_tap(int x, int y)
{
    if (U.setup_step == SU_TOUCH) {
        for (int i = 0; i < 4; i++) {
            int tx, ty;
            setup_target(i, &tx, &ty);
            if ((x - tx) * (x - tx) + (y - ty) * (y - ty) <= 34 * 34 && !(U.touched >> i & 1)) {
                U.touched |= (uint8_t)(1 << i);
                haptic(UI_HAPTIC_COMMIT);
            }
        }
        if (abs(x - CX) <= 70 && abs(y - 190) <= 30)
            setup_next();
        return;
    }
    if (y >= NEW_BTN_Y1 - 8 && y <= NEW_BTN_Y2 + 8) {
        if (x >= 124 && x <= 264)
            setup_next();
        else if (x >= 58 && x < 118) {
            if (U.setup_step)
                U.setup_step--;
            else if (!U.setup_first) { /* left without finishing: what was changed on the way stays */
                settings_save();
                U.scr = UI_SCR_LIST;
            }
        }
    } else if (U.setup_step == SU_NAME && abs(y - SU_NAME_Y) <= 28) {
        if (x < CX)
            setup_knob(1);
        else
            edit_open(ED_NAME, S.name, sizeof(S.name) - 1);
    } else if (y < 240 && (x < 110 || x >= 250))
        setup_knob(x < 110 ? -1 : 1);
}

static void draw_cb(lv_event_t *e)
{
    lv_layer_t *l = lv_event_get_layer(e);
    switch (U.scr) {
    case UI_SCR_BOOT: draw_boot(l); break;
    case UI_SCR_HOME: draw_home(l); break;
    case UI_SCR_NEW: draw_new(l); break;
    case UI_SCR_MARKERS: draw_markers(l); break;
    case UI_SCR_SETTINGS: draw_settings(l); break;
    case UI_SCR_LIST: draw_list(l); break;
    case UI_SCR_EDIT: draw_edit(l); break;
    case UI_SCR_SETUP: draw_setup(l); break;
    case UI_SCR_GAME:
        if (solo())
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

/* Past the logo and the lock: home, or straight back into the saved game. */
static void unlocked(void)
{
    U.home_focus = U.has_resume ? HOME_RESUME : HOME_START;
    U.scr = UI_SCR_HOME;
    if (S.v[SD_S_START_ON] && U.has_resume)
        start_game(&U.resume);
}

/* The logo is over. */
static void boot_done(void)
{
    if (!S.v[SD_S_SETUP_DONE])
        setup_open(true);
    else if (S.pin[0] && S.v[SD_S_PIN_BOOT])
        edit_open(ED_BOOT, NULL, 4);
    else
        unlocked();
}

void ui_set_settings(const sd_settings_t *s)
{
    if (s)
        S = *s;
    else
        sd_settings_defaults(&S);
    s_ready = true;
    if (U.dial) {
        apply_live();
        redraw();
    }
}

const sd_settings_t *ui_settings(void) { return &S; }

void ui_init(const ui_hooks_t *hooks, const sd_profile_t *profiles, int n_profiles, const sd_game_t *resume)
{
    memset(&U, 0, sizeof(U));
    if (!s_ready)
        ui_set_settings(NULL);
    if (hooks)
        U.hooks = *hooks;
    U.profiles = profiles;
    U.n_profiles = n_profiles;
    default_profile();
    if (resume) {
        U.resume = *resume;
        U.has_resume = true;
    }
    /* kerning off: GPOS is hidden from stb (ui/assets/strip_gpos.py), and Barlow has no kern table (Original Motion's is ignored) */
#define TTF(data, len, px) lv_tiny_ttf_create_data_ex(data, len, px, LV_FONT_KERNING_NONE, 64)
    const uint8_t *dsp = T->display_ttf, *lbl = T->label_ttf;
    size_t dl = (size_t)(T->display_ttf_end - dsp), ll = (size_t)(T->label_ttf_end - lbl);
    U.f_solo = TTF(dsp, dl, 200);
    U.f_arena[2] = TTF(dsp, dl, 112);
    U.f_arena[3] = TTF(dsp, dl, 92);
    U.f_arena[4] = TTF(dsp, dl, 80);
    U.f_hub = TTF(dsp, dl, 48);
    U.f_title = TTF(dsp, dl, 64);
    const uint8_t *om = _binary_Original_Motion_ttf_start;
    size_t ol = (size_t)(_binary_Original_Motion_ttf_end - om);
    U.f_name[0] = TTF(om, ol, 64);
    U.f_name[1] = TTF(om, ol, 48);
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

/* One knob step as the screen up understands it; also what a tap on a - / + or a chevron does. */
static void knob(int detents)
{
    if (!detents)
        return;
    switch (U.scr) {
    case UI_SCR_BOOT: boot_done(); break; /* any input skips the logo */
    case UI_SCR_HOME: /* START, REPRENDRE (when there is a game), settings */
        for (int k = abs(detents); k; k--)
            do
                U.home_focus = (U.home_focus + (detents > 0 ? 1 : HOME_N - 1)) % HOME_N;
            while (U.home_focus == HOME_RESUME && !U.has_resume);
        haptic(UI_HAPTIC_DETENT);
        break;
    case UI_SCR_NEW:
        if (U.new_step == NEW_FORMAT) {
            U.profile = ((U.profile + detents) % U.n_profiles + U.n_profiles) % U.n_profiles;
            U.players = U.profiles[U.profile].players;
        } else if (U.new_step == NEW_PLAYERS)
            players_step(detents);
        else if (detents & 1) {
            sd_setting_set(&S, SD_S_VIEW, !S.v[SD_S_VIEW]);
            U.dirty = true;
        }
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
    case UI_SCR_SHEET:
        if (U.sheet_src >= 0) {
            uint8_t p, src;
            sheet_cell(U.sheet_src, &p, &src);
            adjust(p, SD_CMD, src, detents);
        } else
            adjust(U.sel, SD_POISON, U.sel, detents);
        break;
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
    case UI_SCR_SETTINGS:
        U.set_focus = ((U.set_focus + detents) % (SEC_N + 1) + SEC_N + 1) % (SEC_N + 1);
        haptic(UI_HAPTIC_DETENT);
        break;
    case UI_SCR_LIST:
        if (U.editing)
            row_change(cur_row(), detents);
        else { /* the list stops at both ends: the last row is the way back */
            int i = U.row + detents, n = SECTIONS[U.section].n;
            U.row = i < 0 ? 0 : i >= n ? n - 1 : i;
            U.armed = -1;
        }
        haptic(UI_HAPTIC_DETENT);
        break;
    case UI_SCR_EDIT: edit_knob(detents); break;
    case UI_SCR_SETUP: setup_knob(detents); break;
    case UI_SCR_OVER: break;
    }
    redraw();
}

void ui_knob(int detents, uint32_t now)
{
    U.now = now;
    /* where the knob counts points, it takes SD_S_CLICKS detents to make one */
    if (U.scr == UI_SCR_GAME || U.scr == UI_SCR_SHEET || U.scr == UI_SCR_MARKERS)
        detents = sd_clicks_to_points(&U.click_rest, detents, S.v[SD_S_CLICKS]);
    knob(detents);
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
    case MENU_SHEET:
        U.sheet_src = -1;
        U.scr = UI_SCR_SHEET;
        break;
    case MENU_NEW: go_home(); break;
    case MENU_SETTINGS: open_settings(true); break;
    default: U.scr = UI_SCR_GAME; break;
    }
}

void ui_tap(int x, int y, uint32_t now)
{
    U.now = now;
    U.click_rest = 0;
    switch (U.scr) {
    case UI_SCR_BOOT: boot_done(); break;
    case UI_SCR_HOME:
#define IN(cx, cy, r) ((x - (cx)) * (x - (cx)) + (y - (cy)) * (y - (cy)) <= (r) * (r))
        if (IN(CX, NEW_Y, NEW_R))
            home_activate(HOME_START);
        else if (U.has_resume && x >= RESUME_X1 && x <= RESUME_X2 && y >= RESUME_Y1 && y <= RESUME_Y2)
            home_activate(HOME_RESUME);
        else if (IN(92, 96, 32))
            home_activate(HOME_SETTINGS);
        else if (!IN(268, 96, 32) && !IN(76, 206, 42) && !IN(284, 206, 42)) /* the dimmed buttons do nothing */
            home_activate(U.home_focus);
        break;
    case UI_SCR_NEW:
        if (y >= NEW_BTN_Y1 - 8 && y <= NEW_BTN_Y2 + 8) {
            if (x >= 124 && x <= 264) {
                if (new_last()) {
                    settings_save(); /* the game screen chosen here is remembered */
                    start_game(NULL);
                } else
                    U.new_step++;
            } else if (x >= 58 && x < 118) { /* back: one step, then home */
                if (U.new_step == NEW_FORMAT)
                    U.scr = UI_SCR_HOME;
                else
                    U.new_step--;
            }
        } else if (y < 240 && (x < 110 || x >= 250)) {
            knob(x < 110 ? -1 : 1);
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
        } else if (solo()) {
            if (undo_pill() && y >= 276 && y < 324 && x >= 100 && x < 260)
                undo();
            else if (x < 110 || x >= 250)
                knob(x < 110 ? -1 : 1);
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
                    knob(lx < 0 ? -1 : 1);
            }
        }
        break;
    case UI_SCR_MENU:
        for (int i = 0; i < MENU_ROWS; i++)
            if (y >= menu_row_y(i) - 22 && y < menu_row_y(i) + 22) {
                U.menu_focus = i;
                menu_activate(i);
            }
        break;
    case UI_SCR_LOG: U.scr = UI_SCR_MENU; break;
    case UI_SCR_SHEET: {
        int col = -1, cols = U.g.n_players - 1;
        for (int s = 0, k = 0; s < U.g.n_players && y >= 262 && U.g.rules.cmd_lethal; s++)
            if (s != U.sel && abs(x - sheet_col_x(k++, cols)) <= 36)
                col = s;
        flush(); /* a pending tally is committed before the knob changes target or the sheet closes */
        if (U.scr != UI_SCR_SHEET)
            break;
        if (col >= 0)
            U.sheet_src = col == U.sheet_src ? -1 : col; /* a commander column takes the knob; again = poison */
        else
            U.scr = UI_SCR_GAME;
        break;
    }
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
    case UI_SCR_SETTINGS: {
        int hit = (x - CX) * (x - CX) + (y - 304) * (y - 304) <= 30 * 30 ? SET_BACK : -1;
        for (int i = 0; i < SEC_N; i++) {
            int sx, sy;
            settings_sec_xy(i, &sx, &sy);
            if (abs(x - sx) <= 70 && abs(y - sy) <= 26)
                hit = i;
        }
        if (hit >= 0)
            U.set_focus = hit;
        settings_activate(U.set_focus); /* a tap anywhere else runs the focused one, as on the home screen */
        break;
    }
    case UI_SCR_LIST: list_tap(x, y); break;
    case UI_SCR_EDIT: edit_tap(x, y); break;
    case UI_SCR_SETUP: setup_tap(x, y); break;
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
    if (U.scr != UI_SCR_GAME || solo())
        return;
    int a = seat_at(x0, y0), b = seat_at(x1, y1);
    if (a < 0 || b < 0 || a == b)
        return;
    U.sel = (uint8_t)b; /* A attacks B: combat damage, the knob sets the amount, a hub tap makes it commander damage */
    adjust((uint8_t)b, SD_LIFE, (uint8_t)a, -1);
    if (S.v[SD_S_DRAG_CMD])
        sd_game_pending_toggle_cmd(&U.g); /* a no-op in a format without commander damage */
    redraw();
}

void ui_tick(uint32_t now)
{
    U.now = now;
    if (U.scr == UI_SCR_BOOT) { /* the delay starts at the first tick: ui_init() has no clock */
        if (!U.boot_until)
            U.boot_until = now + UI_BOOT_MS;
        else if (now >= U.boot_until) {
            boot_done();
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
