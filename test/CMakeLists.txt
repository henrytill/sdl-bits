enable_testing()

# load_bmp
add_executable(test_load_bmp load_bmp.c)
SET_COMPILER_OPTIONS(test_load_bmp)
target_link_libraries(test_load_bmp PRIVATE bmp)
add_test(NAME load_bmp
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND test_load_bmp)

# buffer
add_executable(test_util_buffer util_buffer.c)
SET_COMPILER_OPTIONS(test_util_buffer)
target_link_libraries(test_util_buffer PRIVATE util)
add_test(NAME buffer
         COMMAND test_util_buffer)

# buffer2d
add_executable(test_util_buffer2d util_buffer2d.c)
SET_COMPILER_OPTIONS(test_util_buffer2d)
target_link_libraries(test_util_buffer2d PRIVATE util)
add_test(NAME buffer2d
         COMMAND test_util_buffer2d)

# uint32_sat_sub
add_executable(test_util_uint32_sat_sub util_uint32_sat_sub.c)
SET_COMPILER_OPTIONS(test_util_uint32_sat_sub)
target_link_libraries(test_util_uint32_sat_sub PRIVATE util)
add_test(NAME uint32_sat_sub
         COMMAND test_util_uint32_sat_sub)
