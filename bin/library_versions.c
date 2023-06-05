#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <SDL.h>
#include <lauxlib.h>
#include <lua.h>

#include "macro.h"

enum {
	APP = SDL_LOG_CATEGORY_CUSTOM
};

static void free_type(void)
{
	FT_Library lib = NULL;
	FT_Int major = 0;
	FT_Int minor = 0;
	FT_Int patch = 0;

	int rc = FT_Init_FreeType(&lib);
	if (rc != 0) {
		SDL_LogError(APP, "Failed to initialize FreeType");
		return;
	}
	FT_Library_Version(lib, &major, &minor, &patch);
	SDL_LogInfo(APP, "Linking against FreeType %u.%u.%u", major, minor, patch);
	FT_Done_FreeType(lib);
}

static void lua(void)
{
	lua_State *state = luaL_newstate();
	if (state == NULL) {
		SDL_LogError(APP, "Failed to initialize Lua");
		return;
	}

	const lua_Number linked = lua_version(state);
	SDL_LogInfo(APP, "Compiled against Lua %s ...", LUA_RELEASE);
	SDL_LogInfo(APP, "... and linking against Lua %.2f", linked);
	lua_close(state);
}

static void sdl(void)
{
	SDL_version compiled = {0};
	SDL_version linked = {0};

	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);
	SDL_LogInfo(APP, "Compiled against SDL %u.%u.%u ...", compiled.major, compiled.minor, compiled.patch);
	SDL_LogInfo(APP, "... and linking against SDL %u.%u.%u.", linked.major, linked.minor, linked.patch);
}

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

	free_type();
	lua();
	sdl();

	return EXIT_SUCCESS;
}
