# Executable: render_font_to_file
add_executable(render_font_to_file render_font_to_file.c)
SET_COMPILER_OPTIONS(render_font_to_file)

if(MSVC)
  target_include_directories(render_font_to_file PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>)
  target_link_libraries(render_font_to_file PRIVATE freetype bmp)
else()
  target_include_directories(render_font_to_file PRIVATE ${FREETYPE2_INCLUDE_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>)
  target_link_libraries(render_font_to_file PRIVATE ${FREETYPE2_LIBRARIES} bmp)
endif()
