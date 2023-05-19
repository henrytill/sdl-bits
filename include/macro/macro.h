#pragma once

#include <stddef.h>
#include <stdio.h>

#ifndef NDEBUG
#define DEBUG
#endif

// C2X compatibility

#ifndef static_assert
#define static_assert _Static_assert
#endif

// General

#ifdef DEBUG
#define debug_printf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define debug_printf(fmt, ...) ({})
#endif

// OOP

#define TYPEOF_MEMBER(type, member) typeof(((type *)0)->member)

#define SAME_TYPE(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))

#define CONTAINER_OF_(ptr, type, member) ({                                        \
  void *__mptr = (void *)(ptr);                                                    \
  static_assert(SAME_TYPE(*(ptr), ((type *)0)->member) || SAME_TYPE(*(ptr), void), \
                "pointer type mismatch");                                          \
  ((type *)(__mptr - offsetof(type, member)));                                     \
})

#ifdef HAS_GENERIC
#define CONTAINER_OF(ptr, type, member)                                       \
  _Generic(                                                                   \
    ptr,                                                                      \
    const typeof(*(ptr)) *: ((const type *)CONTAINER_OF_(ptr, type, member)), \
    default: ((type *)CONTAINER_OF_(ptr, type, member)))
#else
#define CONTAINER_OF CONTAINER_OF_
#endif

#define SEND(obj, method, ...) ({      \
  typeof(obj) __obj = (obj);           \
  __obj->method(__obj, ##__VA_ARGS__); \
})

// Cleanup

#define AT_EXIT(func) ({                           \
  if (atexit(func) != 0) {                         \
    fprintf(stderr, "atexit(%s) failed\n", #func); \
    exit(EXIT_FAILURE);                            \
  }                                                \
})

#define DEFINE_TRIVIAL_CLEANUP_FUNC(type, func)                                  \
  static inline void func##p(type *p) { /* NOLINT(bugprone-macro-parentheses) */ \
    if (*p) {                                                                    \
      func(*p);                                                                  \
      debug_printf("%s(*%p)\n", #func, (void *)p);                               \
    }                                                                            \
  }
