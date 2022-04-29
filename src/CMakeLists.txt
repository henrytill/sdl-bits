# Library: bmp
add_library(bmp bmp.c)
SET_COMPILER_OPTIONS(bmp)
target_compile_definitions(bmp PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(bmp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
  target_link_libraries(bmp PRIVATE m)
endif()

# Library: util
add_library(util util.c)
SET_COMPILER_OPTIONS(util)
target_include_directories(util INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Executable: main
add_executable(main main.c)
SET_COMPILER_OPTIONS(main)
TARGET_LINK_SDL(main)
target_link_libraries(main PRIVATE util)
