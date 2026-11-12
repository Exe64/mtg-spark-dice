#pragma once
#include <stdint.h>

#include "driver/i2c_master.h"

/* DRV2605 library effect ids (calibration knob: pick what feels right on the device) */
#define HAPTIC_TICK 24   /* sharp tick 1: knob detent */
#define HAPTIC_CLICK 1   /* strong click: commit */
#define HAPTIC_BUZZ 14   /* strong buzz: defeat */

esp_err_t haptic_init(i2c_master_bus_handle_t bus);
void haptic_play(uint8_t effect);
