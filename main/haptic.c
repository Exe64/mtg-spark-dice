/* Minimal DRV2605 driver: internal-trigger mode, ERM library 1 (as the Waveshare demo). */
#include "haptic.h"

#include "board.h"
#include "esp_log.h"

static i2c_master_dev_handle_t dev;

static esp_err_t wr(uint8_t reg, uint8_t val)
{
    uint8_t b[2] = {reg, val};
    return i2c_master_transmit(dev, b, 2, 20);
}

esp_err_t haptic_init(i2c_master_bus_handle_t bus)
{
    i2c_device_config_t cfg = {.dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = DRV2605_ADDR, .scl_speed_hz = 400000};
    esp_err_t err = i2c_master_bus_add_device(bus, &cfg, &dev);
    if (err == ESP_OK)
        err = wr(0x01, 0x00); /* MODE: internal trigger, leave standby */
    if (err == ESP_OK)
        err = wr(0x03, 0x01); /* LIBRARY: ERM A */
    if (err != ESP_OK) {
        ESP_LOGW("haptic", "DRV2605 not responding (%s), haptics disabled", esp_err_to_name(err));
        dev = NULL;
    }
    return err;
}

void haptic_play(uint8_t effect)
{
    if (!dev)
        return;
    wr(0x04, effect); /* WAV_SEQ1 */
    wr(0x05, 0);      /* end of sequence */
    wr(0x0C, 1);      /* GO */
}
