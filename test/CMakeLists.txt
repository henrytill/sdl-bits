enable_testing()

add_executable(test_load_bmp load_bmp.c)
SET_COMPILER_OPTIONS(test_load_bmp)
target_include_directories(test_load_bmp PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>)
target_link_libraries(test_load_bmp PRIVATE bmp)

add_executable(test_util_buffer util_buffer.c test.h)
SET_COMPILER_OPTIONS(test_util_buffer)
target_include_directories(test_util_buffer PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>)
target_link_libraries(test_util_buffer PRIVATE util)

add_executable(test_util_buffer2d util_buffer2d.c test.h)
SET_COMPILER_OPTIONS(test_util_buffer2d)
target_include_directories(test_util_buffer2d PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>)
target_link_libraries(test_util_buffer2d PRIVATE util)

add_executable(test_util_uint32_sat_sub util_uint32_sat_sub.c test.h)
SET_COMPILER_OPTIONS(test_util_uint32_sat_sub)
target_include_directories(test_util_uint32_sat_sub PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>)
target_link_libraries(test_util_uint32_sat_sub PRIVATE util)

add_test(buffer test_util_buffer)
add_test(buffer2d test_util_buffer2d)
add_test(uint32_sat_sub test_util_uint32_sat_sub)
add_test(NAME load_bmp WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND test_load_bmp)
