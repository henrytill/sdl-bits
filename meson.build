project(
  'sdl-bits',
  'c',
  version: '0.1',
  default_options: ['warning_level=2', 'c_std=gnu11'],
)

# dependencies
cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required: true)

freetype_dep = dependency('freetype2', required: true)

sdl_dep = dependency('sdl2', required: true)

thread_dep = dependency('threads', required: true)

# subproject dependencies
cmake = import('cmake')
lua_subproj = cmake.subproject('lua')
lua_dep = lua_subproj.dependency('lua')

# libraries

macro_include_dir = include_directories('include/macro')

macro_dep = declare_dependency(include_directories: macro_include_dir)

prelude_include_dir = include_directories('include/prelude')

prelude_dep = declare_dependency(
  include_directories: prelude_include_dir,
  dependencies: [sdl_dep],
)

bmp_include_dir = include_directories('include/bmp')

bmp = static_library(
  'bmp',
  'lib/bmp/bmp.c',
  include_directories: [bmp_include_dir],
  dependencies: [m_dep],
  install: false,
)

bmp_dep = declare_dependency(
  link_with: [bmp],
  include_directories: [bmp_include_dir],
)

message_queue_include_dir = include_directories('include/message_queue')

message_queue_sdl = static_library(
  'message_queue_sdl',
  'lib/message_queue/message_queue_sdl.c',
  include_directories: message_queue_include_dir,
  dependencies: [sdl_dep, macro_dep],
  install: false,
)

message_queue_sdl_dep = declare_dependency(
  link_with: [message_queue_sdl],
  include_directories: message_queue_include_dir,
)

message_queue_posix = static_library(
  'message_queue_posix',
  'lib/message_queue/message_queue_posix.c',
  include_directories: message_queue_include_dir,
  dependencies: [thread_dep, macro_dep],
  install: false,
)

message_queue_posix_dep = declare_dependency(
  link_with: [message_queue_posix],
  include_directories: message_queue_include_dir,
)

# executables
executable(
  'generate_atlas_from_bdf',
  'bin/generate_atlas_from_bdf.c',
  dependencies: [freetype_dep, bmp_dep],
  install: false,
)

generate_test_bmp = executable(
  'generate_test_bmp',
  'bin/generate_test_bmp.c',
  dependencies: [bmp_dep],
  install: false,
)

executable(
  'get_displays',
  'bin/get_displays.c',
  dependencies: [macro_dep, sdl_dep],
  install: false,
)

executable(
  'library_versions',
  'bin/library_versions.c',
  dependencies: [macro_dep, freetype_dep, lua_dep, sdl_dep],
  install: false,
)

executable(
  'main',
  'bin/main.c',
  dependencies: [prelude_dep, macro_dep, lua_dep, m_dep, message_queue_sdl_dep],
  install: true,
)

# files
test_bmp = custom_target(
  'test_bmp',
  output: 'test.bmp',
  command: [generate_test_bmp, '@OUTPUT@'],
)

# tests
bmp_read_bitmap = executable(
  'bmp_read_bitmap',
  'test/bmp_read_bitmap.c',
  dependencies: [bmp_dep],
  install: false,
)

bmp_read_bitmap_v4 = executable(
  'bmp_read_bitmap_v4',
  'test/bmp_read_bitmap_v4.c',
  dependencies: [bmp_dep],
  install: false,
)

message_queue_basic = executable(
  'message_queue_basic',
  'test/message_queue_basic.c',
  dependencies: [prelude_dep, macro_dep, message_queue_sdl_dep],
  install: false,
)

message_queue_copies = executable(
  'message_queue_copies',
  'test/message_queue_copies.c',
  dependencies: [prelude_dep, macro_dep, message_queue_sdl_dep],
  install: false,
)

message_queue_basic_posix = executable(
  'message_queue_basic_posix',
  'test/message_queue_basic.c',
  dependencies: [prelude_dep, macro_dep, message_queue_posix_dep],
  install: false,
)

message_queue_copies_posix = executable(
  'message_queue_copies_posix',
  'test/message_queue_copies.c',
  dependencies: [prelude_dep, macro_dep, message_queue_posix_dep],
  install: false,
)

test('bmp_read_bitmap', bmp_read_bitmap, args: ['../assets/sample_24bit.bmp'])

test(
  'bmp_read_bitmap_v4',
  bmp_read_bitmap_v4,
  args: ['test.bmp'],
  depends: test_bmp,
)

test('message_queue_basic', message_queue_basic)
test('message_queue_copies', message_queue_copies)
test('message_queue_basic_posix', message_queue_basic_posix)
test('message_queue_copies_posix', message_queue_copies_posix)
