/* Waveshare ESP32-S3-Knob-Touch-LCD-1.8 pinout.
 * Source: Waveshare demo pins as reproduced by community ports (FlightScnr waveshare_knob_18.h, jovlinger/utils
 * spinme notes). NOT yet verified on hardware: check against the wiki schematic when the boards arrive. */
#pragma once

#define PIN_LCD_SCLK 13
#define PIN_LCD_CS 14
#define PIN_LCD_D0 15
#define PIN_LCD_D1 16
#define PIN_LCD_D2 17
#define PIN_LCD_D3 18
#define PIN_LCD_RST 21
#define PIN_LCD_BL 47

#define PIN_I2C_SDA 11 /* shared: CST816 touch + DRV2605 haptic */
#define PIN_I2C_SCL 12
#define PIN_TOUCH_INT 9
#define PIN_TOUCH_RST 10
#define DRV2605_ADDR 0x5A

#define PIN_KNOB_A 8
#define PIN_KNOB_B 7
/* Quadrature counts per mechanical detent and the knob's direction are settings (SD_S_KNOB_COUNTS, SD_S_KNOB_REVERSE):
 * the first-start assistant measures them, the device settings change them. */

/* Touch gestures */
#define TOUCH_DRAG_PX 24
#define TOUCH_LONG_MS 600
