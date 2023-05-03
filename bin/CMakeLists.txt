# demo_oop
add_executable(demo_oop demo_oop.c)
target_compile_options_default(demo_oop)
target_link_libraries(demo_oop PRIVATE macro)

# generate_atlas_from_bdf
add_executable(generate_atlas_from_bdf generate_atlas_from_bdf.c)
target_compile_options_default(generate_atlas_from_bdf)
if(WIN32)
  target_compile_definitions(generate_atlas_from_bdf PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
target_link_libraries(generate_atlas_from_bdf PRIVATE bmp)
target_link_libraries(generate_atlas_from_bdf PRIVATE macro)
target_link_freetype(generate_atlas_from_bdf)

# generate_test_bmp
add_executable(generate_test_bmp generate_test_bmp.c)
target_compile_options_default(generate_test_bmp)
target_link_libraries(generate_test_bmp PRIVATE bmp)
target_link_libraries(generate_test_bmp PRIVATE macro)

# get_displays
add_executable(get_displays get_displays.c)
target_compile_options_default(get_displays)
target_link_libraries(get_displays PRIVATE macro)
target_link_SDL2(get_displays PRIVATE)

# library_versions
add_executable(library_versions library_versions.c)
target_compile_options_default(library_versions)
target_link_libraries(library_versions PRIVATE macro)
target_link_SDL2(library_versions PRIVATE)
target_link_freetype(library_versions)
target_link_lua(library_versions PRIVATE)

# main
add_executable(main main.c)
target_compile_options_default(main)
target_link_libraries(main PRIVATE prelude)
target_link_SDL2(main PRIVATE)
target_link_lua(main PRIVATE)
