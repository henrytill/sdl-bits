# Highest version compatible with Steam Runtime container
cmake_minimum_required(VERSION 3.13)
include(CheckSymbolExists)

project(sdl-bits C)

set(CMAKE_C_STANDARD 99)

# https://stackoverflow.com/a/58711983
string(REGEX REPLACE "/W3" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

macro(SET_COMPILER_OPTIONS target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()
endmacro(SET_COMPILER_OPTIONS)

# https://blog.kitware.com/cmake-and-the-default-build-type/
set(default_build_type "Debug")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")

  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Dependency: SDL
find_package(SDL2 CONFIG REQUIRED)

# Dependency: Freetype
if(MSVC)
  find_package(freetype CONFIG REQUIRED)
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(FREETYPE2 REQUIRED freetype2)
endif()

# assets
add_subdirectory(assets)

# src
add_subdirectory(src)

# test
add_subdirectory(test)

# tool
add_subdirectory(tool)
