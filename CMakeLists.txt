# Highest version compatible with Steam Runtime container
cmake_minimum_required(VERSION 3.13)

project(sdl-bits C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

include(CheckSymbolExists)

# https://gitlab.kitware.com/cmake/cmake/-/issues/21730
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
include(CTest)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(BuildType)
include(Coverage)
include(FeatureTest)

test_has_attribute_cleanup()
test_has_builtin_types_compatible_p()

if(HAS_ATTRIBUTE_CLEANUP)
  message(STATUS "Compiler supports __attribute__((cleanup(f)))")
else()
  message(FATAL_ERROR "Compiler does not support __attribute__((cleanup(f)))")
endif()

if(HAS_BUILTIN_TYPES_COMPATIBLE_P)
  message(STATUS "Compiler supports __builtin_types_compatible_p")
else()
  message(FATAL_ERROR "Compiler does not support __builtin_types_compatible_p")
endif()

check_symbol_exists(alloca alloca.h stdlib.h HAVE_ALLOCA)

# https://stackoverflow.com/a/58711983
string(REGEX REPLACE "/W3" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# For dependency management, we use vcpkg.
# If that's not available, we use the default package manager and pkg-config for Linux and MinGW.
set(VCPKG_FOUND ${VCPKG_TOOLCHAIN} CACHE INTERNAL "")

if((NOT VCPKG_FOUND) AND (UNIX OR MINGW))
  message(STATUS "Using default package manager and pkg-config")
  find_package(PkgConfig REQUIRED)
elseif(VCPKG_FOUND)
  message(STATUS "Using vcpkg")
else()
  message(FATAL_ERROR "Missing vcpkg toolchain file argument")
endif()

# Dependency: m
if(UNIX)
  list(APPEND CMAKE_REQUIRED_LIBRARIES "m")
endif()
check_symbol_exists(ceil math.h HAVE_CEIL)

# Dependency: SDL
find_package(SDL2 CONFIG REQUIRED)

# Dependency: Freetype
if(VCPKG_FOUND)
  find_package(freetype CONFIG REQUIRED)
elseif(PKG_CONFIG_FOUND)
  pkg_check_modules(FREETYPE2 REQUIRED freetype2)
endif()

# Dependency: Lua
add_subdirectory(third-party/lua)

# Macros
macro(target_compile_options_default target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion)
  endif()
endmacro(target_compile_options_default)

macro(target_link_SDL2 target scope)
  # On MinGW builds, this call to target_link_libraries must be before the call for SDL2::SDL2main
  if(MINGW)
    target_link_libraries(${target} ${scope} mingw32)
  endif()
  if(BUILD_SHARED_LIBS)
    target_link_libraries(${target} ${scope} SDL2::SDL2main SDL2::SDL2)
  else()
    target_link_libraries(${target} ${scope} SDL2::SDL2main SDL2::SDL2-static)
  endif()
endmacro(target_link_SDL2)

macro(target_link_freetype target)
  if(VCPKG_FOUND)
    target_link_libraries(${target} PRIVATE freetype)
  elseif(PKG_CONFIG_FOUND AND FREETYPE2_FOUND)
    target_include_directories(${target} PRIVATE ${FREETYPE2_INCLUDE_DIRS})
    if(BUILD_SHARED_LIBS)
      target_link_libraries(${target} PRIVATE ${FREETYPE2_LIBRARIES})
    else()
      target_link_libraries(${target} PRIVATE ${FREETYPE2_STATIC_LIBRARIES})
    endif()
  endif()
endmacro(target_link_freetype)

macro(target_link_lua target scope)
  target_link_libraries(${target} ${scope} lua)
endmacro(target_link_lua)

set(SDL_BITS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(assets)
add_subdirectory(bin)
add_subdirectory(lib)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
