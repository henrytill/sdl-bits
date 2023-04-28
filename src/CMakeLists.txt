# Library: bmp
add_library(bmp OBJECT bmp.c)
target_compile_options_default(bmp)
if(WIN32)
  target_compile_definitions(bmp PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
target_include_directories(bmp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
  target_link_libraries(bmp PRIVATE m)
endif()

add_library(msgq OBJECT msgq.c)
target_compile_options_default(msgq)
target_link_SDL2(msgq)
target_include_directories(msgq INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Executable: main
add_executable(main main.c)
target_compile_options_default(main)
target_link_SDL2(main)
target_link_lua(main)
