# Library: bmp
add_library(bmp bmp.h bmp.c)
SET_COMPILER_OPTIONS(bmp)

check_symbol_exists(ceil math.h HAVE_CEIL)
if(NOT HAVE_CEIL)
  target_link_libraries(bmp PRIVATE m)
endif()

# Library: util
add_library(util util.h util.c)
SET_COMPILER_OPTIONS(util)

# Executable: main
add_executable(main main.c)
SET_COMPILER_OPTIONS(main)

if(MINGW)
  target_link_libraries(main PRIVATE util mingw32 SDL2::SDL2main SDL2::SDL2)
else()
  target_link_libraries(main PRIVATE util SDL2::SDL2main SDL2::SDL2)
endif()
