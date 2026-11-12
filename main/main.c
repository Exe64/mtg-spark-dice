/* SparkDice firmware: hardware glue (display, touch gestures, knob, haptics, NVS, LittleFS) around ui/ + core/. */
#include <string.h>

#include "board.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/pulse_cnt.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st77916.h"
#include "esp_lcd_touch_cst816s.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/semphr.h"
#include "lvgl.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "game.h"
#include "haptic.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "profile.h"
#include "theme.h"
#include "ui.h"

static const char *TAG = "sparkdice";
static nvs_handle_t nvs;
static esp_lcd_touch_handle_t touch;
static pcnt_unit_handle_t knob;
static SemaphoreHandle_t lv_mutex;
static esp_lcd_panel_handle_t panel;

static uint32_t now_ms(void) { return (uint32_t)(esp_timer_get_time() / 1000); }

/* ---------- persistence ---------- */

static void save_game(const sd_game_t *g)
{
    static uint8_t blob[sizeof(sd_game_t) + 16];
    size_t n = sd_game_save(g, blob, sizeof(blob));
    if (n && nvs_set_blob(nvs, "game", blob, n) == ESP_OK)
        nvs_commit(nvs);
}

static bool load_game(sd_game_t *g)
{
    static uint8_t blob[sizeof(sd_game_t) + 16];
    size_t n = sizeof(blob);
    return nvs_get_blob(nvs, "game", blob, &n) == ESP_OK && sd_game_load(g, blob, n);
}

static int load_profiles(sd_profile_t *out, int max)
{
    static const char *files[] = {"/storage/commander.json", "/storage/standard.json"};
    int n = 0;
    for (size_t i = 0; i < sizeof(files) / sizeof(files[0]) && n < max; i++) {
        const char *err = "";
        if (sd_profile_load(files[i], &out[n], &err))
            n++;
        else
            ESP_LOGE(TAG, "profile %s: %s", files[i], err);
    }
    return n;
}

static void on_haptic(ui_haptic_t p)
{
    haptic_play(p == UI_HAPTIC_DETENT ? HAPTIC_TICK : p == UI_HAPTIC_COMMIT ? HAPTIC_CLICK : HAPTIC_BUZZ);
}

/* ---------- hardware init ---------- */

/* ---------- display + LVGL glue (no esp_lvgl_port: 2.9.0 needs LVGL >= 9.3) ---------- */

static bool disp_lock(void) { return xSemaphoreTakeRecursive(lv_mutex, portMAX_DELAY) == pdTRUE; }
static void disp_unlock(void) { xSemaphoreGiveRecursive(lv_mutex); }

static bool on_flush_done(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *e, void *ctx)
{
    (void)io;
    (void)e;
    lv_display_flush_ready((lv_display_t *)ctx);
    return false;
}

static void flush_cb(lv_display_t *d, const lv_area_t *a, uint8_t *px)
{
    (void)d;
    lv_draw_sw_rgb565_swap(px, lv_area_get_size(a)); /* panel expects big-endian RGB565 */
    esp_lcd_panel_draw_bitmap(panel, a->x1, a->y1, a->x2 + 1, a->y2 + 1, px);
}

/* QSPI panel writes need even-aligned windows */
static void rounder_cb(lv_event_t *e)
{
    lv_area_t *a = lv_event_get_param(e);
    a->x1 &= ~1;
    a->y1 &= ~1;
    a->x2 |= 1;
    a->y2 |= 1;
}

static void lvgl_task(void *arg)
{
    (void)arg;
    for (;;) {
        uint32_t wait = 10;
        if (disp_lock()) {
            wait = lv_timer_handler();
            disp_unlock();
        }
        vTaskDelay(pdMS_TO_TICKS(wait < 5 ? 5 : wait > 30 ? 30 : wait));
    }
}

static void display_init(void)
{
    lv_mutex = xSemaphoreCreateRecursiveMutex();
    lv_init();
    lv_tick_set_cb(now_ms);
    lv_display_t *disp = lv_display_create(UI_W, UI_H);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    const size_t buf_px = UI_W * 40;
    void *b1 = heap_caps_malloc(buf_px * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    void *b2 = heap_caps_malloc(buf_px * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    assert(b1 && b2);
    lv_display_set_buffers(disp, b1, b2, buf_px * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_add_event_cb(disp, rounder_cb, LV_EVENT_INVALIDATE_AREA, NULL);

    spi_bus_config_t bus = ST77916_PANEL_BUS_QSPI_CONFIG(PIN_LCD_SCLK, PIN_LCD_D0, PIN_LCD_D1, PIN_LCD_D2, PIN_LCD_D3,
                                                         buf_px * sizeof(uint16_t));
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO));
    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_io_spi_config_t io_cfg = ST77916_PANEL_IO_QSPI_CONFIG(PIN_LCD_CS, on_flush_done, disp);
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_cfg, &io));
    /* ponytail: driver default init sequence; use the Waveshare demo's init_cmds if the panel stays blank */
    st77916_vendor_config_t vendor = {.flags = {.use_qspi_interface = 1}};
    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = PIN_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .vendor_config = &vendor,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st77916(io, &panel_cfg, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));
    gpio_set_direction(PIN_LCD_BL, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_LCD_BL, 1); /* ponytail: full brightness, LEDC dimming with sleep (lot 2) */
    xTaskCreatePinnedToCore(lvgl_task, "lvgl", 8192, NULL, 5, NULL, 1); /* tiny_ttf rasterises in this task */
}

static i2c_master_bus_handle_t i2c_init(void)
{
    i2c_master_bus_config_t cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PIN_I2C_SDA,
        .scl_io_num = PIN_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &bus));
    return bus;
}

static void touch_init(i2c_master_bus_handle_t bus)
{
    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_io_i2c_config_t io_cfg = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
    io_cfg.scl_speed_hz = 400000;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(bus, &io_cfg, &io));
    esp_lcd_touch_config_t cfg = {
        .x_max = UI_W,
        .y_max = UI_H,
        .rst_gpio_num = PIN_TOUCH_RST,
        .int_gpio_num = PIN_TOUCH_INT,
    };
    if (esp_lcd_touch_new_i2c_cst816s(io, &cfg, &touch) != ESP_OK)
        ESP_LOGE(TAG, "CST816 init failed, touch disabled");
}

static void knob_init(void)
{
    pcnt_unit_config_t ucfg = {.low_limit = -1000, .high_limit = 1000};
    ESP_ERROR_CHECK(pcnt_new_unit(&ucfg, &knob));
    pcnt_glitch_filter_config_t fcfg = {.max_glitch_ns = 1000};
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(knob, &fcfg));
    pcnt_chan_config_t a = {.edge_gpio_num = PIN_KNOB_A, .level_gpio_num = PIN_KNOB_B};
    pcnt_chan_config_t b = {.edge_gpio_num = PIN_KNOB_B, .level_gpio_num = PIN_KNOB_A};
    pcnt_channel_handle_t ca, cb;
    ESP_ERROR_CHECK(pcnt_new_channel(knob, &a, &ca));
    ESP_ERROR_CHECK(pcnt_new_channel(knob, &b, &cb));
    /* standard x4 quadrature decoding */
    pcnt_channel_set_edge_action(ca, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    pcnt_channel_set_level_action(ca, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    pcnt_channel_set_edge_action(cb, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE);
    pcnt_channel_set_level_action(cb, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    gpio_pullup_en(PIN_KNOB_A);
    gpio_pullup_en(PIN_KNOB_B);
    ESP_ERROR_CHECK(pcnt_unit_enable(knob));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(knob));
    ESP_ERROR_CHECK(pcnt_unit_start(knob));
}

/* ---------- input loop ---------- */

/* Returns whole detents turned since the last call; keeps the remainder. */
static int knob_read(void)
{
    static int last;
    int count;
    pcnt_unit_get_count(knob, &count);
    int detents = (count - last) / KNOB_COUNTS_PER_DETENT;
    last += detents * KNOB_COUNTS_PER_DETENT;
    if (last > 800 || last < -800) { /* re-centre before the hardware limit */
        pcnt_unit_clear_count(knob);
        last = 0;
    }
    return detents;
}

typedef struct {
    bool down, long_fired, dragging;
    int x0, y0, x, y;
    uint32_t t0;
} touch_state_t;

/* Turns raw touch samples into tap / long press / drag. */
static void touch_poll(touch_state_t *s, uint32_t now)
{
    esp_lcd_touch_point_data_t pt = {0};
    uint8_t cnt = 0;
    if (touch) {
        esp_lcd_touch_read_data(touch);
        esp_lcd_touch_get_data(touch, &pt, &cnt, 1);
    }
    uint16_t x = pt.x, y = pt.y;
    if (cnt) {
        if (!s->down)
            *s = (touch_state_t){.down = true, .x0 = x, .y0 = y, .t0 = now};
        s->x = x;
        s->y = y;
        int dx = s->x - s->x0, dy = s->y - s->y0;
        if (dx * dx + dy * dy > TOUCH_DRAG_PX * TOUCH_DRAG_PX)
            s->dragging = true;
        if (!s->dragging && !s->long_fired && now - s->t0 >= TOUCH_LONG_MS) {
            s->long_fired = true;
            ui_long_press(s->x0, s->y0, now);
        }
    } else if (s->down) {
        if (s->dragging)
            ui_drag(s->x0, s->y0, s->x, s->y, now);
        else if (!s->long_fired)
            ui_tap(s->x0, s->y0, now);
        s->down = false;
    }
}

static void input_task(void *arg)
{
    (void)arg;
    touch_state_t ts = {0};
    for (;;) {
        uint32_t now = now_ms();
        if (disp_lock()) {
            int d = knob_read();
            if (d)
                ui_knob(d, now);
            touch_poll(&ts, now);
            ui_tick(now);
            disp_unlock();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(nvs_open("sparkdice", NVS_READWRITE, &nvs));
    esp_vfs_littlefs_conf_t fs = {.base_path = "/storage", .partition_label = "storage"};
    ESP_ERROR_CHECK(esp_vfs_littlefs_register(&fs));

    static sd_profile_t profiles[4];
    int n_profiles = load_profiles(profiles, 4);
    static sd_game_t saved;
    bool has_saved = load_game(&saved);
    ESP_LOGI(TAG, "%d profiles, saved game: %s", n_profiles, has_saved ? "yes" : "no");

    i2c_master_bus_handle_t bus = i2c_init();
    touch_init(bus);
    haptic_init(bus);
    knob_init();
    display_init();

    disp_lock();
    /* look: stored apart from the game; written by the settings screens (plan #3) */
    uint8_t theme = UI_THEME_DEFAULT, ground = 0;
    nvs_get_u8(nvs, "theme", &theme);
    nvs_get_u8(nvs, "ground", &ground);
    ui_set_theme(theme, ground);
    ui_hooks_t hooks = {.haptic = on_haptic, .save = save_game};
    ui_init(&hooks, profiles, n_profiles, has_saved ? &saved : NULL);
    disp_unlock();

    xTaskCreate(input_task, "input", 6144, NULL, 4, NULL);
}
