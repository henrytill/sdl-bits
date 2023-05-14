#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_audio.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "macro.h"

#define now SDL_GetPerformanceCounter

/// Log categories to use with SDL logging functions.
enum LogCategory {
  APP = SDL_LOG_CATEGORY_CUSTOM,
  ERR,
};

// General cleanup functions

static inline void freestr(char *str) {
  if (str != NULL)
    free(str);
}

DEFINE_TRIVIAL_CLEANUP_FUNC(char *, freestr)
#define _cleanup_str_ __attribute__((cleanup(freestrp)))

// SDL cleanup functions

DEFINE_TRIVIAL_CLEANUP_FUNC(SDL_Surface *, SDL_FreeSurface)
DEFINE_TRIVIAL_CLEANUP_FUNC(SDL_Texture *, SDL_DestroyTexture)
DEFINE_TRIVIAL_CLEANUP_FUNC(SDL_AudioDeviceID, SDL_CloseAudioDevice)
#define _cleanup_SDL_Surface_       _cleanup_(SDL_FreeSurfacep)
#define _cleanup_SDL_Texture_       _cleanup_(SDL_DestroyTexturep)
#define _cleanup_SDL_AudioDeviceID_ _cleanup_(SDL_CloseAudioDevicep)

// Lua cleanup functions

DEFINE_TRIVIAL_CLEANUP_FUNC(lua_State *, lua_close)
#define _cleanup_lua_State_ _cleanup_(lua_closep)

// Let's not use SDL-defined numeric types

static_assert(__builtin_types_compatible_p(Uint16, uint16_t), "SDL-defined Uint16 is not uint16_t");
static_assert(__builtin_types_compatible_p(Uint32, uint32_t), "SDL-defined Uint32 is not uint32_t");
static_assert(__builtin_types_compatible_p(Uint64, uint64_t), "SDL-defined Uint64 is not uint64_t");
static_assert(__builtin_types_compatible_p(Sint16, int16_t), "SDL-defined Sint16 is not int16_t");
static_assert(__builtin_types_compatible_p(Sint32, int32_t), "SDL-defined Sint32 is not int32_t");
static_assert(__builtin_types_compatible_p(Sint64, int64_t), "SDL-defined Sint64 is not int64_t");
static_assert(__builtin_types_compatible_p(SDL_AudioFormat, uint16_t), "SDL-defined SDL_AudioFormat is not uint16_t");
static_assert(__builtin_types_compatible_p(SDL_AudioDeviceID, uint32_t), "SDL-defined SDL_AudioDeviceID is not uint32_t");

// General utility functions

#define ALLOCATION_FAILURE_MSG "Failed to allocate.\n"

///
/// Allocate or die.
///
/// @param size The size in bytes to allocate.
/// @return A pointer to the allocated memory.
///
static inline void *emalloc(size_t size) {
  void *ret = malloc(size);
  if (ret == NULL) {
    fprintf(stderr, ALLOCATION_FAILURE_MSG);
    exit(EXIT_FAILURE);
  }
  return ret;
}

///
/// Allocate and zero or die.
///
/// @param nmemb The number of elements to allocate.
/// @param size The size in bytes of each element.
/// @return A pointer to the allocated memory.
///
static inline void *ecalloc(size_t nmemb, size_t size) {
  void *ret = calloc(nmemb, size);
  if (ret == NULL) {
    fprintf(stderr, ALLOCATION_FAILURE_MSG);
    exit(EXIT_FAILURE);
  }
  return ret;
}

// SDL utility functions

///
/// Log a message and the contents of SDL_GetError().
///
/// @param msg The message to log
///
static inline void sdl_error(const char *msg) {
  const char *err = SDL_GetError();
  if (strlen(err) != 0)
    SDL_LogError(ERR, "%s (%s)", msg, err);
  else
    SDL_LogError(ERR, "%s", msg);
}
