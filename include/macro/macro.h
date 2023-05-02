#pragma once

#include <stddef.h>

#ifndef NDEBUG
#define DEBUG
#endif

#ifndef static_assert
#define static_assert _Static_assert
#endif

#define _cleanup_(f) __attribute__((cleanup(f)))
#define _packed_     __attribute__((packed))
#define _unused_     __attribute__((unused))

#define array_size(array) (sizeof(array) / sizeof((array)[0]))

#define typeof_member(type, member) typeof(((type *)0)->member)

#define same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))

#define container_of(ptr, type, member) ({                                         \
  void *__mptr = (void *)(ptr);                                                    \
  static_assert(same_type(*(ptr), ((type *)0)->member) || same_type(*(ptr), void), \
                "pointer type mismatch");                                          \
  ((type *)(__mptr - offsetof(type, member)));                                     \
})

#define container_of_const(ptr, type, member)                                \
  _Generic(                                                                  \
    ptr,                                                                     \
    const typeof(*(ptr)) *: ((const type *)container_of(ptr, type, member)), \
    default: ((type *)container_of(ptr, type, member)))

#define send(obj, method, ...) ({    \
  typeof(obj) _obj = (obj);          \
  _obj->method(_obj, ##__VA_ARGS__); \
})

#ifdef DEBUG
#define debug_printf(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define debug_printf(fmt, ...) ({})
#endif

#define exitwith(func) ({                          \
  if (atexit(func) != 0) {                         \
    fprintf(stderr, "atexit(%s) failed\n", #func); \
    exit(EXIT_FAILURE);                            \
  }                                                \
})

#define DEFINE_TRIVIAL_CLEANUP_FUNC(type, func)    \
  static inline void func##p(type *p) {            \
    if (*p) {                                      \
      func(*p);                                    \
      debug_printf("%s(*%p)\n", #func, (void *)p); \
    }                                              \
  }                                                \
  static_assert(1, "end of DEFINE_TRIVIAL_CLEANUP_FUNC")
