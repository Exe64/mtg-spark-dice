idf_component_register(SRCS "ui.c" "theme_booster.c" "theme_lumineux.c" "skull_a8.c" "fx.c" "fx_a8.c" "logo_a8.c" "wordmark_a8.c"
                       INCLUDE_DIRS "."
                       REQUIRES core profiles lvgl
                       EMBED_FILES "assets/BarlowCondensed-ExtraBold.ttf" "assets/BarlowCondensed-SemiBold.ttf")
