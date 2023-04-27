# produce_consume
add_executable(produce_consume produce_consume.c)
target_compile_options_default(produce_consume)
target_link_SDL2(produce_consume)
target_link_libraries(produce_consume PRIVATE queue)
add_test(NAME produce_consume
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND produce_consume)


# read_bitmap
add_executable(read_bitmap read_bitmap.c)
target_compile_options_default(read_bitmap)
target_link_libraries(read_bitmap PRIVATE bmp)
add_test(NAME read_bitmap
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND read_bitmap)

# read_bitmap_v4
add_executable(read_bitmap_v4 read_bitmap_v4.c)
target_compile_options_default(read_bitmap_v4)
target_link_libraries(read_bitmap_v4 PRIVATE bmp)
add_test(NAME read_bitmap_v4
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND read_bitmap_v4)
add_dependencies(read_bitmap_v4 test_bmp)
