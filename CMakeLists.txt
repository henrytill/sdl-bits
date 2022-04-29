# Highest version compatible with Steam Runtime container
cmake_minimum_required(VERSION 3.13)
include(CheckSymbolExists)
enable_testing()

project(sdl-bits C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
include(BuildType)

# https://stackoverflow.com/a/58711983
string(REGEX REPLACE "/W3" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

macro(SET_COMPILER_OPTIONS target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion)
  endif()
endmacro(SET_COMPILER_OPTIONS)

macro(TARGET_LINK_SDL target)
  # On MinGW builds, this call to target_link_libraries must be before the call for SDL2::SDL2main
  if(MINGW)
    target_link_libraries(${target} PRIVATE mingw32)
  endif()
  target_link_libraries(${target} PRIVATE SDL2::SDL2main SDL2::SDL2)
endmacro(TARGET_LINK_SDL)

# For dependency management, we use vcpkg.
# If that's not available, we use the default package manager and pkg-config for Linux and MinGW.
set(VCPKG_FOUND ${VCPKG_TOOLCHAIN} CACHE INTERNAL "")

if((NOT VCPKG_FOUND) AND (UNIX OR MINGW))
  find_package(PkgConfig REQUIRED)
elseif(NOT VCPKG_FOUND)
  message(FATAL_ERROR "Missing vcpkg toolchain file argument")
endif()

# Dependency: m
if(UNIX)
  list(APPEND CMAKE_REQUIRED_LIBRARIES "m")
endif()

check_symbol_exists(ceil math.h HAVE_CEIL)

# Dependency: SDL
find_package(SDL2 CONFIG REQUIRED)

# Dependency: SDL_ttf
if(VCPKG_FOUND)
  find_package(sdl2-ttf CONFIG REQUIRED)
elseif(PKG_CONFIG_FOUND)
  pkg_check_modules(SDL2_ttf REQUIRED SDL2_ttf)
endif()

# Dependency: Freetype
if(VCPKG_FOUND)
  find_package(freetype CONFIG REQUIRED)
elseif(PKG_CONFIG_FOUND)
  pkg_check_modules(FREETYPE2 REQUIRED freetype2)
endif()

# Directory: assets
add_subdirectory(assets)

# Directory: src
add_subdirectory(src)

# Directory: test
add_subdirectory(test)

# Directory: tool
add_subdirectory(tool)
