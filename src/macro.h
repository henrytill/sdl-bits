#ifndef SDL_BITS_MACRO_H
#define SDL_BITS_MACRO_H

#ifndef NDEBUG
#define DEBUG
#endif

#ifndef static_assert
#define static_assert _Static_assert
#endif

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) \
  do {                        \
  } while (0)
#endif

#define ATEXIT(func)                                 \
  do {                                               \
    if (atexit(func) != 0) {                         \
      fprintf(stderr, "atexit(%s) failed\n", #func); \
      exit(EXIT_FAILURE);                            \
    }                                                \
  } while (0)

#define _cleanup_(f) __attribute__((cleanup(f)))

#define DEFINE_TRIVIAL_CLEANUP_FUNC(type, func)   \
  static inline void func##p(type *p) {           \
    if (*p) {                                     \
      func(*p);                                   \
      DEBUG_PRINT("%s(*%p)\n", #func, (void *)p); \
    }                                             \
  }                                               \
  static_assert(1, "end of DEFINE_TRIVIAL_CLEANUP_FUNC")

#endif /* SDL_BITS_MACRO_H */