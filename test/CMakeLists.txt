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

# message_queue_basic
add_executable(message_queue_basic message_queue_basic.c)
target_compile_options_default(message_queue_basic)
target_link_libraries(message_queue_basic PRIVATE message_queue)
target_link_libraries(message_queue_basic PRIVATE prelude)
target_link_SDL2(message_queue_basic PRIVATE)
add_test(NAME message_queue_basic
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND message_queue_basic)

# message_queue_copies
add_executable(message_queue_copies message_queue_copies.c)
target_compile_options_default(message_queue_copies)
target_link_libraries(message_queue_copies PRIVATE message_queue)
target_link_libraries(message_queue_copies PRIVATE prelude)
target_link_SDL2(message_queue_copies PRIVATE)
add_test(NAME message_queue_copies
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
         COMMAND message_queue_copies)

if (TARGET message_queue_posix)
  # message_queue_basic_posix
  add_executable(message_queue_basic_posix message_queue_basic.c)
  target_compile_options_default(message_queue_basic_posix)
  target_link_libraries(message_queue_basic_posix PRIVATE message_queue_posix)
  target_link_libraries(message_queue_basic_posix PRIVATE prelude)
  add_test(NAME message_queue_basic_posix
           WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
           COMMAND message_queue_basic_posix)

  # message_queue_copies_posix
  add_executable(message_queue_copies_posix message_queue_copies.c)
  target_compile_options_default(message_queue_copies_posix)
  target_link_libraries(message_queue_copies_posix PRIVATE message_queue_posix)
  target_link_libraries(message_queue_copies_posix PRIVATE prelude)
  add_test(NAME message_queue_copies_posix
           WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
           COMMAND message_queue_copies_posix)
endif()
