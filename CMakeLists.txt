# Highest version compatible with Steam Runtime container
cmake_minimum_required(VERSION 3.13)

project(sdl-bits)

set(CMAKE_C_STANDARD 90)

# https://blog.kitware.com/cmake-and-the-default-build-type/
set(default_build_type "Debug")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")

  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

find_package(SDL2 CONFIG REQUIRED)

if(MSVC)
  find_package(freetype CONFIG REQUIRED)
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(FREETYPE2 REQUIRED freetype2)
endif()

add_subdirectory(assets)

set(SDL_BITS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SDL_BITS_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

set(
  FONT_LOADER_SOURCES
  ${SDL_BITS_SOURCE_DIR}/font_loader.c
)
add_executable(font_loader ${FONT_LOADER_SOURCES})

if(MSVC)
  target_link_libraries(font_loader PRIVATE freetype)
else()
  target_include_directories(font_loader PRIVATE ${FREETYPE2_INCLUDE_DIRS})
  target_link_libraries(font_loader PRIVATE ${FREETYPE2_LIBRARIES})
endif()

set(
  BMP_SOURCES
  ${SDL_BITS_SOURCE_DIR}/bmp.h
  ${SDL_BITS_SOURCE_DIR}/bmp.c
)
add_library(bmp ${BMP_SOURCES})
if(CMAKE_COMPILER_IS_GNUCXX)
  target_link_libraries(bmp PRIVATE m)
endif()

set(
  UTIL_SOURCES
  ${SDL_BITS_SOURCE_DIR}/util.h
  ${SDL_BITS_SOURCE_DIR}/util.c
)
add_library(util ${UTIL_SOURCES})

set(
  MAIN_SOURCES
  ${SDL_BITS_SOURCE_DIR}/main.c
)
add_executable(main ${MAIN_SOURCES})

if(MINGW)
  target_link_libraries(main PRIVATE util mingw32 SDL2::SDL2main SDL2::SDL2)
else()
  target_link_libraries(main PRIVATE util SDL2::SDL2main SDL2::SDL2)
endif()

if(MSVC)
  target_compile_options(main PRIVATE /W4 /WX)
else()
  target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()

add_executable(test_load_bmp ${SDL_BITS_TEST_DIR}/load_bmp.c)
target_include_directories(test_load_bmp PRIVATE $<BUILD_INTERFACE:${SDL_BITS_SOURCE_DIR}>)
target_link_libraries(test_load_bmp PRIVATE bmp)

add_executable(test_util_buffer ${SDL_BITS_TEST_DIR}/util_buffer.c ${SDL_BITS_TEST_DIR}/test.h)
target_include_directories(test_util_buffer PRIVATE $<BUILD_INTERFACE:${SDL_BITS_SOURCE_DIR}>)
target_link_libraries(test_util_buffer PRIVATE util)

add_executable(test_util_buffer2d ${SDL_BITS_TEST_DIR}/util_buffer2d.c ${SDL_BITS_TEST_DIR}/test.h)
target_include_directories(test_util_buffer2d PRIVATE $<BUILD_INTERFACE:${SDL_BITS_SOURCE_DIR}>)
target_link_libraries(test_util_buffer2d PRIVATE util)

add_executable(test_util_uint32_sat_sub ${SDL_BITS_TEST_DIR}/util_uint32_sat_sub.c ${SDL_BITS_TEST_DIR}/test.h)
target_include_directories(test_util_uint32_sat_sub PRIVATE $<BUILD_INTERFACE:${SDL_BITS_SOURCE_DIR}>)
target_link_libraries(test_util_uint32_sat_sub PRIVATE util)

add_test(buffer test_util_buffer)
add_test(buffer2d test_util_buffer2d)
add_test(uint32_sat_sub test_util_uint32_sat_sub)
add_test(NAME load_bmp WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND test_load_bmp)