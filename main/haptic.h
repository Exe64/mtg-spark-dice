#pragma once
#include <stdint.h>

#include "driver/i2c_master.h"

/* DRV2605 library effect ids, by intensity (SD_S_HAPTIC 1..3) and pattern (detent, commit, defeat).
 * Calibration knob: pick what feels right on the device. */
#define HAPTIC_EFFECTS \
    { \
        {26, 3, 50}, /* light: sharp tick 3 (60 %), strong click 30 %, buzz 4 (40 %) */ \
        {25, 2, 48}, /* medium: sharp tick 2 (80 %), strong click 60 %, buzz 2 (80 %) */ \
        {24, 1, 14}, /* strong: sharp tick 1, strong click, strong buzz (100 %) */ \
    }

esp_err_t haptic_init(i2c_master_bus_handle_t bus);
void haptic_play(uint8_t effect);
