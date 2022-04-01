find_package(Python3)

# generate.py targets
set(TARGETS ucs-fonts dummy)

foreach(TARGET ${TARGETS})
  STRING(MAKE_C_IDENTIFIER ${TARGET} TARGET_NAME)

  SET(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET})

  add_custom_command(OUTPUT ${OUTPUT_PATH}
                     COMMAND ${Python3_EXECUTABLE} ARGS generate.py ${TARGET_NAME}
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  add_custom_target(${TARGET_NAME} ALL DEPENDS ${OUTPUT_PATH})

  set_target_properties(${TARGET_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${OUTPUT_PATH})
endforeach()

# render_bdf_font_to_file
set(FONT_BMP_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/10x20.bmp)

add_custom_command(OUTPUT ${FONT_BMP_OUTPUT_PATH}
                   COMMAND render_bdf_font_to_file
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   DEPENDS render_bdf_font_to_file ucs_fonts)

add_custom_target(font_bmp ALL DEPENDS ${FONT_BMP_OUTPUT_PATH})

set_target_properties(font_bmp PROPERTIES ADDITIONAL_CLEAN_FILES ${FONT_BMP_OUTPUT_PATH})

# render_test_bitmap
set(TEST_BMP_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/test.bmp)

add_custom_command(OUTPUT ${TEST_BMP_OUTPUT_PATH}
                   COMMAND render_test_bitmap
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   DEPENDS render_test_bitmap)

add_custom_target(test_bmp ALL DEPENDS ${TEST_BMP_OUTPUT_PATH})

set_target_properties(test_bmp PROPERTIES ADDITIONAL_CLEAN_FILES ${TEST_BMP_OUTPUT_PATH})
