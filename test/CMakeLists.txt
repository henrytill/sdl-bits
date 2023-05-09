# bmp_read_bitmap
add_executable(bmp_read_bitmap bmp_read_bitmap.c)
target_compile_options_default(bmp_read_bitmap)
target_link_libraries(bmp_read_bitmap PRIVATE bmp)
target_link_libraries(bmp_read_bitmap PRIVATE prelude)
add_test(NAME bmp_read_bitmap
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND bmp_read_bitmap)

# bmp_read_bitmap_v4
add_executable(bmp_read_bitmap_v4 bmp_read_bitmap_v4.c)
target_compile_options_default(bmp_read_bitmap_v4)
target_link_libraries(bmp_read_bitmap_v4 PRIVATE bmp)
target_link_libraries(bmp_read_bitmap_v4 PRIVATE prelude)
add_test(NAME bmp_read_bitmap_v4
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND bmp_read_bitmap_v4)
add_dependencies(bmp_read_bitmap_v4 test_bmp)

# msgq_basic
add_executable(msgq_basic msgq_basic.c)
target_compile_options_default(msgq_basic)
target_link_libraries(msgq_basic PRIVATE msgq)
target_link_libraries(msgq_basic PRIVATE prelude)
target_link_SDL2(msgq_basic PRIVATE)
add_test(NAME msgq_basic
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND msgq_basic)

# msgq_copies
add_executable(msgq_copies msgq_copies.c)
target_compile_options_default(msgq_copies)
target_link_libraries(msgq_copies PRIVATE msgq)
target_link_libraries(msgq_copies PRIVATE prelude)
target_link_SDL2(msgq_copies PRIVATE)
add_test(NAME msgq_copies
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND msgq_copies)
