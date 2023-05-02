#pragma once

#ifndef NDEBUG
#define DEBUG
#endif

#ifndef static_assert
#define static_assert _Static_assert
#endif

#define _cleanup_(f) __attribute__((cleanup(f)))
#define _packed_     __attribute__((packed))
#define _unused_     __attribute__((unused))

#ifdef DEBUG
#define debug_printf(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define debug_printf(fmt, ...) \
  do {                         \
  } while (0)
#endif

#define exitwith(func)                               \
  do {                                               \
    if (atexit(func) != 0) {                         \
      fprintf(stderr, "atexit(%s) failed\n", #func); \
      exit(EXIT_FAILURE);                            \
    }                                                \
  } while (0)

#define DEFINE_TRIVIAL_CLEANUP_FUNC(type, func)    \
  static inline void func##p(type *p) {            \
    if (*p) {                                      \
      func(*p);                                    \
      debug_printf("%s(*%p)\n", #func, (void *)p); \
    }                                              \
  }                                                \
  static_assert(1, "end of DEFINE_TRIVIAL_CLEANUP_FUNC")
