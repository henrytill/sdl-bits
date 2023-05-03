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

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#define TYPEOF_MEMBER(type, member) typeof(((type *)0)->member)

#define SAME_TYPE(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))

#define CONTAINER_OF(ptr, type, member) ({                                         \
  void *__mptr = (void *)(ptr);                                                    \
  static_assert(SAME_TYPE(*(ptr), ((type *)0)->member) || SAME_TYPE(*(ptr), void), \
                "pointer type mismatch");                                          \
  ((type *)(__mptr - offsetof(type, member)));                                     \
})

#define CONTAINER_OF_CONST(ptr, type, member)                                \
  _Generic(                                                                  \
    ptr,                                                                     \
    const typeof(*(ptr)) *: ((const type *)CONTAINER_OF(ptr, type, member)), \
    default: ((type *)CONTAINER_OF(ptr, type, member)))

#define SEND(obj, method, ...) ({    \
  typeof(obj) _obj = (obj);          \
  _obj->method(_obj, ##__VA_ARGS__); \
})

#ifdef DEBUG
#define debug_printf(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define debug_printf(fmt, ...) ({})
#endif

#define AT_EXIT(func) ({                           \
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
  }
