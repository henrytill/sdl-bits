#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <SDL.h>
#include <lauxlib.h>
#include <lua.h>

#include "macro.h"

enum {
  UNHANDLED = SDL_LOG_CATEGORY_CUSTOM
};

static void freetype(void) {
  FT_Library lib = NULL;
  FT_Int major;
  FT_Int minor;
  FT_Int patch;
  int rc;

  rc = FT_Init_FreeType(&lib);
  if (rc != 0) {
    SDL_LogError(UNHANDLED, "Failed to initialize FreeType");
    return;
  }
  FT_Library_Version(lib, &major, &minor, &patch);
  SDL_LogInfo(UNHANDLED, "Linking against FreeType %u.%u.%u", major, minor, patch);
  FT_Done_FreeType(lib);
}

static void lua(void) {
  lua_State *state = luaL_newstate();
  if (state == NULL) {
    SDL_LogError(UNHANDLED, "Failed to initialize Lua");
    return;
  }
  const lua_Number linked = lua_version(state);
  SDL_LogInfo(UNHANDLED, "Compiled against Lua %s ...", LUA_RELEASE);
  SDL_LogInfo(UNHANDLED, "... and linking against Lua %.2f", linked);
  lua_close(state);
}

static void sdl(void) {
  SDL_version compiled;
  SDL_version linked;

  SDL_VERSION(&compiled);
  SDL_GetVersion(&linked);
  SDL_LogInfo(UNHANDLED, "Compiled against SDL %u.%u.%u ...", compiled.major, compiled.minor, compiled.patch);
  SDL_LogInfo(UNHANDLED, "... and linking against SDL %u.%u.%u.", linked.major, linked.minor, linked.patch);
}

int main(_unused_ int argc, _unused_ char *argv[]) {
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

  freetype();
  lua();
  sdl();
  return EXIT_SUCCESS;
}
