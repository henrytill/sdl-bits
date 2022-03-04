cmake_minimum_required(VERSION 3.16)
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

set(SDL_BITS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SDL_BITS_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

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
target_link_libraries(main PRIVATE util SDL2::SDL2 SDL2::SDL2main)

if(MSVC)
  target_compile_options(main PRIVATE /W4 /WX)
else()
  target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()

add_executable(util-buffer-test ${SDL_BITS_TEST_DIR}/util-buffer-test.c)
target_include_directories(util-buffer-test PRIVATE $<BUILD_INTERFACE:${SDL_BITS_SOURCE_DIR}>)
target_link_libraries(util-buffer-test PRIVATE util)

add_test(buffer_test, util-buffer-test)