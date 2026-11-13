# Host (PC) build: game logic tests + headless UI screenshots.
#   cmake -S host -B host/build -G Ninja && cmake --build host/build && ctest --test-dir host/build
cmake_minimum_required(VERSION 3.16)
project(sparkdice_host C)
set(CMAKE_C_STANDARD 11)
add_compile_options(-Wall -Wextra -Werror)
set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
enable_testing()

add_library(sd_core ${ROOT}/core/game.c)
target_include_directories(sd_core PUBLIC ${ROOT}/core)
add_executable(test_core test_core.c)
target_link_libraries(test_core sd_core)
add_test(NAME core COMMAND test_core)

# Third-party sources are cloned by the operator into host/third_party/ (see plan prerequisites).
# cJSON: on target it comes from the IDF `json` component.
set(TP ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
add_library(cjson ${TP}/cJSON/cJSON.c)
target_include_directories(cjson PUBLIC ${TP}/cJSON)
target_compile_options(cjson PRIVATE -w)

add_library(sd_profiles ${ROOT}/profiles/profile.c)
target_include_directories(sd_profiles PUBLIC ${ROOT}/profiles)
target_link_libraries(sd_profiles PUBLIC sd_core cjson)
add_executable(test_profiles test_profiles.c)
target_link_libraries(test_profiles sd_profiles)
target_compile_definitions(test_profiles PRIVATE PROFILES_DIR="${ROOT}/profiles/data")
add_test(NAME profiles COMMAND test_profiles)

# LVGL 9.2 (headless), configured by host/lv_conf.h
file(GLOB_RECURSE LVGL_SRC ${TP}/lvgl/src/*.c)
add_library(lvgl ${LVGL_SRC})
target_include_directories(lvgl PUBLIC ${TP}/lvgl ${TP}/lvgl/src ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(lvgl PUBLIC LV_CONF_INCLUDE_SIMPLE)
target_compile_options(lvgl PRIVATE -w)
target_link_libraries(lvgl PUBLIC m)

add_library(shot shot.c)
target_link_libraries(shot PUBLIC lvgl)
add_executable(test_shot test_shot.c)
target_link_libraries(test_shot shot)
add_test(NAME shot COMMAND test_shot ${CMAKE_CURRENT_BINARY_DIR}/test_shot.png)

# UI (shared with firmware) + embedded fonts. Fonts become _binary_<name>_ttf_start/_end symbols, as IDF EMBED_FILES does.
include(${ROOT}/ui/display_font.cmake)
set(FONTS "${SD_DISPLAY_TTF}" ${ROOT}/ui/assets/BarlowCondensed-SemiBold.ttf)
set(FONT_OBJS)
foreach(f ${FONTS})
  get_filename_component(name "${f}" NAME_WE)
  string(MAKE_C_IDENTIFIER "${name}" sym)
  set(obj ${CMAKE_CURRENT_BINARY_DIR}/${sym}.o)
  add_custom_command(OUTPUT ${obj}
    COMMAND cp "${f}" ${CMAKE_CURRENT_BINARY_DIR}/${sym}.ttf
    COMMAND ld -r -b binary -z noexecstack -o ${obj} ${sym}.ttf
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS "${f}")
  list(APPEND FONT_OBJS ${obj})
endforeach()
add_library(sd_ui ${ROOT}/ui/ui.c ${ROOT}/ui/theme_booster.c ${ROOT}/ui/theme_lumineux.c ${ROOT}/ui/skull_a8.c ${ROOT}/ui/fx.c ${ROOT}/ui/fx_a8.c ${ROOT}/ui/logo_a8.c ${ROOT}/ui/wordmark_a8.c ${ROOT}/ui/home_icons_a8.c ${FONT_OBJS})
target_include_directories(sd_ui PUBLIC ${ROOT}/ui)
target_link_libraries(sd_ui PUBLIC sd_profiles lvgl m)

add_executable(test_fx test_fx.c)
target_link_libraries(test_fx sd_ui shot)
add_test(NAME fx COMMAND test_fx ${CMAKE_CURRENT_BINARY_DIR}/shots-out/fx-specimen.png)

# not a test (timings): host/build/bench <profiles_dir>
add_executable(bench bench.c)
target_link_libraries(bench sd_ui shot)

add_executable(shots shots.c)
target_link_libraries(shots sd_ui shot)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shots-out/booster)
add_test(NAME ui_flows COMMAND shots ${ROOT}/profiles/data ${CMAKE_CURRENT_BINARY_DIR}/shots-out)
add_test(NAME ui_flows_booster COMMAND shots ${ROOT}/profiles/data ${CMAKE_CURRENT_BINARY_DIR}/shots-out/booster booster)
add_custom_target(screenshots COMMAND ${CMAKE_COMMAND} -E make_directory ${ROOT}/shots
  COMMAND shots ${ROOT}/profiles/data ${ROOT}/shots DEPENDS shots)
