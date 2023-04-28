#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <SDL.h>

#include "lauxlib.h"
#include "lua.h"

enum {
  UNHANDLED = SDL_LOG_CATEGORY_CUSTOM
};

static void freetype(void) {
  FT_Library ftlib = NULL;
  FT_Int major;
  FT_Int minor;
  FT_Int patch;
  int rc;

  rc = FT_Init_FreeType(&ftlib);
  if (rc != 0) {
    SDL_LogError(UNHANDLED, "Failed to initialize FreeType");
    return;
  }
  FT_Library_Version(ftlib, &major, &minor, &patch);
  SDL_LogInfo(UNHANDLED, "Linking against FreeType %u.%u.%u", major, minor, patch);
  FT_Done_FreeType(ftlib);
}

static void lua(void) {
  lua_State *state = luaL_newstate();
  if (state == NULL) {
    SDL_LogError(UNHANDLED, "Failed to initialize Lua");
    return;
  }
  const lua_Number lded = lua_version(state);
  SDL_LogInfo(UNHANDLED, "Compiled against Lua %s ...", LUA_RELEASE);
  SDL_LogInfo(UNHANDLED, "... and linking against Lua %.2f", lded);
  lua_close(state);
}

static void sdl(void) {
  SDL_version cced;
  SDL_version lded;

  SDL_VERSION(&cced);
  SDL_GetVersion(&lded);
  SDL_LogInfo(UNHANDLED, "Compiled against SDL %u.%u.%u ...", cced.major, cced.minor, cced.patch);
  SDL_LogInfo(UNHANDLED, "... and linking against SDL %u.%u.%u.", lded.major, lded.minor, lded.patch);
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

  freetype();
  lua();
  sdl();
  return EXIT_SUCCESS;
}
