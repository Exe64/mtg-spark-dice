# Format-name font: Original Motion (the product name's face), free for personal use only, so not committed.
# Shared by the firmware (ui/CMakeLists.txt) and the PC build (host/CMakeLists.txt). See ui/assets/README.md.
set(SD_NAME_TTF "${CMAKE_CURRENT_LIST_DIR}/assets/licensed/Original Motion.ttf")
if(NOT EXISTS "${SD_NAME_TTF}")
  message(FATAL_ERROR "Missing ${SD_NAME_TTF}: download Original Motion from https://www.dafont.com/original-motion.font "
                      "into ui/assets/licensed/, then run: python3 ui/assets/strip_gpos.py \"${SD_NAME_TTF}\"")
endif()
# The table directory sits in the first bytes; "GPOS" (47504f53) still there means strip_gpos.py was not run
file(READ "${SD_NAME_TTF}" _head LIMIT 512 HEX)
string(FIND "${_head}" "47504f53" _gpos)
if(_gpos GREATER -1)
  message(FATAL_ERROR "${SD_NAME_TTF} still has a GPOS table (LVGL 9.2.2 reads past it): "
                      "run python3 ui/assets/strip_gpos.py \"${SD_NAME_TTF}\"")
endif()
