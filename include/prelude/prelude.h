#pragma once

#include <stdint.h>

#include <SDL.h>
#include <SDL_audio.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "macro.h"

#define now SDL_GetPerformanceCounter

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

/* Let's not use SDL-defined numeric types */

static_assert(__builtin_types_compatible_p(Uint16, uint16_t), "SDL-defined Uint16 is not uint16_t");
static_assert(__builtin_types_compatible_p(Uint32, uint32_t), "SDL-defined Uint32 is not uint32_t");
static_assert(__builtin_types_compatible_p(Uint64, uint64_t), "SDL-defined Uint64 is not uint64_t");
static_assert(__builtin_types_compatible_p(Sint16, int16_t), "SDL-defined Sint16 is not int16_t");
static_assert(__builtin_types_compatible_p(Sint32, int32_t), "SDL-defined Sint32 is not int32_t");
static_assert(__builtin_types_compatible_p(Sint64, int64_t), "SDL-defined Sint64 is not int64_t");
static_assert(__builtin_types_compatible_p(SDL_AudioFormat, uint16_t), "SDL-defined SDL_AudioFormat is not uint16_t");
static_assert(__builtin_types_compatible_p(SDL_AudioDeviceID, uint32_t), "SDL-defined SDL_AudioDeviceID is not uint32_t");
