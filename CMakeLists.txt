cmake_minimum_required(VERSION 3.19)
project(sdl-bits)

set(CMAKE_C_STANDARD 99)

# https://blog.kitware.com/cmake-and-the-default-build-type/
set(default_build_type "Debug")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")

  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(SDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/SDL")
add_subdirectory(${SDL_DIR})

set(SDL_BITS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SDL_BITS_SOURCES ${SDL_BITS_SOURCE_DIR}/main.c)

add_executable(main ${SDL_BITS_SOURCES})
target_link_libraries(main SDL2main SDL2-static)