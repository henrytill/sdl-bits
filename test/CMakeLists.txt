# read_bitmap
add_executable(test_read_bitmap read_bitmap.c)
target_compile_options_default(test_read_bitmap)
target_link_libraries(test_read_bitmap PRIVATE bmp)
add_test(NAME read_bitmap
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND test_read_bitmap)

# read_bitmap_v4
add_executable(test_read_bitmap_v4 read_bitmap_v4.c)
target_compile_options_default(test_read_bitmap_v4)
target_link_libraries(test_read_bitmap_v4 PRIVATE bmp)
add_test(NAME read_bitmap_v4
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND test_read_bitmap_v4)
add_dependencies(test_read_bitmap_v4 test_bmp)
