# read_bitmap
add_executable(test_read_bitmap read_bitmap.c)
SET_COMPILER_OPTIONS(test_read_bitmap)
target_link_libraries(test_read_bitmap PRIVATE bmp)
add_test(NAME read_bitmap
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND test_read_bitmap)

# read_bitmap_v4
add_executable(test_read_bitmap_v4 read_bitmap_v4.c)
SET_COMPILER_OPTIONS(test_read_bitmap_v4)
target_link_libraries(test_read_bitmap_v4 PRIVATE bmp)
add_test(NAME read_bitmap_v4
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND test_read_bitmap_v4)
add_dependencies(test_read_bitmap_v4 test_bmp)

# buffer
add_executable(test_buffer buffer.c)
SET_COMPILER_OPTIONS(test_buffer)
target_link_libraries(test_buffer PRIVATE buffer)
add_test(NAME buffer
         COMMAND test_buffer)

# buffer2d
add_executable(test_buffer2d buffer2d.c)
SET_COMPILER_OPTIONS(test_buffer2d)
target_link_libraries(test_buffer2d PRIVATE buffer)
add_test(NAME buffer2d
         COMMAND test_buffer2d)

# uint32_sat_sub
add_executable(test_util_uint32_sat_sub util_uint32_sat_sub.c)
SET_COMPILER_OPTIONS(test_util_uint32_sat_sub)
target_link_libraries(test_util_uint32_sat_sub PRIVATE util)
add_test(NAME uint32_sat_sub
         COMMAND test_util_uint32_sat_sub)
