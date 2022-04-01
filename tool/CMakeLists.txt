# Executable: library_versions
add_executable(library_versions library_versions.c)
SET_COMPILER_OPTIONS(library_versions)

# On MinGW builds, this call to target_link_libraries must be before the call for SDL2::SDL2main
if(MINGW)
  target_link_libraries(library_versions PRIVATE mingw32)
endif()

target_link_libraries(library_versions PRIVATE SDL2::SDL2main SDL2::SDL2)

if(VCPKG_FOUND)
  target_link_libraries(library_versions PRIVATE freetype)
elseif(PKG_CONFIG_FOUND AND FREETYPE2_FOUND)
  target_include_directories(library_versions PRIVATE ${FREETYPE2_INCLUDE_DIRS})
  target_link_libraries(library_versions PRIVATE ${FREETYPE2_LIBRARIES})
endif()


# Executable: render_bdf_font_to_file
add_executable(render_bdf_font_to_file render_bdf_font_to_file.c)
SET_COMPILER_OPTIONS(render_bdf_font_to_file)
target_compile_definitions(render_bdf_font_to_file PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(render_bdf_font_to_file PRIVATE bmp)

if(VCPKG_FOUND)
  target_link_libraries(render_bdf_font_to_file PRIVATE freetype)
elseif(PKG_CONFIG_FOUND AND FREETYPE2_FOUND)
  target_include_directories(render_bdf_font_to_file PRIVATE ${FREETYPE2_INCLUDE_DIRS})
  target_link_libraries(render_bdf_font_to_file PRIVATE ${FREETYPE2_LIBRARIES})
endif()


# Executable: render_test_bitmap
add_executable(render_test_bitmap render_test_bitmap.c)
SET_COMPILER_OPTIONS(render_test_bitmap)
target_link_libraries(render_test_bitmap PRIVATE bmp)
