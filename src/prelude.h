#ifndef SDL_BITS_PRELUDE_H
#define SDL_BITS_PRELUDE_H

#include <stdint.h>

#include <SDL.h>
#include <SDL_audio.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

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

/* General cleanup functions */

static inline void freestr(char *str) {
  if (str != NULL)
    free(str);
}

DEFINE_TRIVIAL_CLEANUP_FUNC(char *, freestr);
#define _cleanup_str_ __attribute__((cleanup(freestrp)))

/* SDL cleanup functions */

DEFINE_TRIVIAL_CLEANUP_FUNC(SDL_Surface *, SDL_FreeSurface);
DEFINE_TRIVIAL_CLEANUP_FUNC(SDL_Texture *, SDL_DestroyTexture);
DEFINE_TRIVIAL_CLEANUP_FUNC(SDL_AudioDeviceID, SDL_CloseAudioDevice);
#define _cleanup_SDL_Surface_       _cleanup_(SDL_FreeSurfacep)
#define _cleanup_SDL_Texture_       _cleanup_(SDL_DestroyTexturep)
#define _cleanup_SDL_AudioDeviceID_ _cleanup_(SDL_CloseAudioDevicep)

/* Lua cleanup functions */

DEFINE_TRIVIAL_CLEANUP_FUNC(lua_State *, lua_close);
#define _cleanup_lua_State_ _cleanup_(lua_closep)

#endif /* SDL_BITS_PRELUDE_H */
