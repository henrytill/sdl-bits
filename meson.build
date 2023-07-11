project(
  'sdl-bits', 'c',
  version : '0.1',
  default_options : ['warning_level=2', 'c_std=gnu11'])

# dependencies
cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : true)

freetype_dep = dependency('freetype2', required : true)

sdl_dep = dependency('sdl2', required : true)

thread_dep = dependency('threads', required : true)

# subproject dependencies
cmake = import('cmake')
lua_subproj = cmake.subproject('lua')
lua_dep = lua_subproj.dependency('lua')

# libraries
bmp = library(
  'bmp', 'lib/bmp/bmp.c',
  include_directories : ['include/bmp'],
  dependencies : [m_dep],
  install : false)

message_queue_sdl = library(
  'message_queue_sdl', 'lib/message_queue/message_queue_sdl.c',
  include_directories : ['include/message_queue', 'include/macro'],
  dependencies : [sdl_dep],
  install : false)

message_queue_posix = library(
  'message_queue_posix', 'lib/message_queue/message_queue_posix.c',
  include_directories : ['include/message_queue', 'include/macro'],
  dependencies : [thread_dep],
  install : false)

# executables
executable(
  'generate_atlas_from_bdf', 'bin/generate_atlas_from_bdf.c',
  include_directories : ['include/bmp'],
  link_with : [bmp],
  dependencies : [freetype_dep],
  install : false)

generate_test_bmp = executable(
  'generate_test_bmp', 'bin/generate_test_bmp.c',
  include_directories : ['include/bmp'],
  link_with : [bmp],
  install : false)

executable(
  'get_displays', 'bin/get_displays.c',
  include_directories : ['include/macro'],
  dependencies : [sdl_dep],
  install : false)

executable(
  'library_versions', 'bin/library_versions.c',
  include_directories : ['include/macro'],
  dependencies : [freetype_dep, lua_dep, sdl_dep],
  install : false)

executable(
  'main', 'bin/main.c',
  include_directories : ['include/macro', 'include/prelude'],
  dependencies : [lua_dep, m_dep, sdl_dep],
  install : true)

# files
test_bmp = custom_target(
  'test_bmp',
  output : 'test.bmp',
  command : [generate_test_bmp, '@OUTPUT@'])

# tests
bmp_read_bitmap = executable(
  'bmp_read_bitmap', 'test/bmp_read_bitmap.c',
  include_directories : ['include/bmp'],
  link_with : [bmp],
  install : false)

bmp_read_bitmap_v4 = executable(
  'bmp_read_bitmap_v4', 'test/bmp_read_bitmap_v4.c',
  include_directories : ['include/bmp'],
  link_with : [bmp],
  install : false)

message_queue_basic = executable(
  'message_queue_basic', 'test/message_queue_basic.c',
  include_directories : ['include/message_queue', 'include/macro', 'include/prelude'],
  link_with : [message_queue_sdl],
  dependencies : [sdl_dep],
  install : false)

message_queue_copies = executable(
  'message_queue_copies', 'test/message_queue_copies.c',
  include_directories : ['include/message_queue', 'include/macro', 'include/prelude'],
  link_with : [message_queue_sdl],
  dependencies : [sdl_dep],
  install : false)

message_queue_basic_posix = executable(
  'message_queue_basic_posix', 'test/message_queue_basic.c',
  include_directories : ['include/message_queue', 'include/macro', 'include/prelude'],
  link_with : [message_queue_posix],
  dependencies : [sdl_dep],
  install : false)

message_queue_copies_posix = executable(
  'message_queue_copies_posix', 'test/message_queue_copies.c',
  include_directories : ['include/message_queue', 'include/macro', 'include/prelude'],
  link_with : [message_queue_posix],
  dependencies : [sdl_dep],
  install : false)

test(
  'bmp_read_bitmap', bmp_read_bitmap,
  args : ['../assets/sample_24bit.bmp'])

test(
  'bmp_read_bitmap_v4', bmp_read_bitmap_v4,
  args : ['test.bmp'], depends: test_bmp)

test('message_queue_basic', message_queue_basic)
test('message_queue_copies', message_queue_copies)
test('message_queue_basic_posix', message_queue_basic_posix)
test('message_queue_copies_posix', message_queue_copies_posix)
