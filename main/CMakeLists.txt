idf_component_register(SRCS "main.c" "haptic.c"
                       INCLUDE_DIRS "."
                       REQUIRES core profiles ui nvs_flash esp_driver_gpio esp_driver_i2c esp_driver_pcnt
                                esp_driver_spi esp_lcd esp_timer esp_app_format)
# LittleFS image of the JSON profiles, flashed into the `storage` partition
littlefs_create_partition_image(storage ../profiles/data FLASH_IN_PROJECT)
