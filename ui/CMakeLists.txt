idf_component_register(SRCS "ui.c" "theme_booster.c"
                       INCLUDE_DIRS "."
                       REQUIRES core profiles lvgl
                       EMBED_FILES "assets/BarlowCondensed-ExtraBold.ttf" "assets/BarlowCondensed-SemiBold.ttf")
