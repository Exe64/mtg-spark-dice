include(${CMAKE_CURRENT_LIST_DIR}/name_font.cmake)
idf_component_register(SRCS "ui.c" "strings.c" "strings_fr.c" "strings_en.c" "theme_booster.c" "theme_lumineux.c" "skull_a8.c" "fx.c" "fx_a8.c" "logo_a8.c" "wordmark_a8.c" "home_icons_a8.c"
                       INCLUDE_DIRS "."
                       REQUIRES core profiles lvgl
                       EMBED_FILES "assets/BarlowCondensed-ExtraBold.ttf" "assets/BarlowCondensed-SemiBold.ttf" "${SD_NAME_TTF}")
