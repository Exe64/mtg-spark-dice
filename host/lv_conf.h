/* LVGL config for host (headless screenshots). Target config lives in sdkconfig (Kconfig). */
#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH 32 /* host only: screenshots in XRGB8888; target renders RGB565 */
#define LV_USE_OS LV_OS_NONE
#define LV_MEM_SIZE (4 * 1024 * 1024)
#define LV_DEF_REFR_PERIOD 16

#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF 1

#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_48 1

#define LV_GRADIENT_MAX_STOPS 3 /* foil sheen; mirror in sdkconfig.defaults */
#define LV_USE_LODEPNG 1 /* encoder used by shot.c */
#define LV_USE_TINY_TTF 1
#define LV_TINY_TTF_FILE_SUPPORT 0

#endif
