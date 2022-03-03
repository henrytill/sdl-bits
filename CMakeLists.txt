cmake_minimum_required(VERSION 3.16)
project(sdl-bits)

set(CMAKE_C_STANDARD 90)

# https://stackoverflow.com/questions/10671916/how-to-copy-dll-files-into-the-same-folder-as-the-executable-using-cmake
if(WIN32)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif(WIN32)

# https://blog.kitware.com/cmake-and-the-default-build-type/
set(default_build_type "Debug")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")

  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

include(FetchContent)
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.0.20
)
set(SDL_SHARED ON)
set(SDL_STATIC OFF)
FetchContent_MakeAvailable(SDL2)

set(SDL_BITS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SDL_BITS_SOURCES ${SDL_BITS_SOURCE_DIR}/main.c ${SDL_BITS_SOURCE_DIR}/util.h)

add_executable(main ${SDL_BITS_SOURCES})
target_link_libraries(main SDL2::SDL2main SDL2::SDL2)

if(MSVC)
  target_compile_options(main PRIVATE /W4 /WX)
else()
  target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
