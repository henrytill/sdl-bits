#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

enum {
	SUCCESS = 0,
	FAILURE = 1,
};

enum {
	UNHANDLED = SDL_LOG_CATEGORY_CUSTOM
};

enum Loopstat {
	STOP = 0,
	RUN = 1
};

struct Args {
	char *cfgfile;
};

struct Config {
	enum {
		WINDOWED = 0,
		FULLSCREEN = 1,
		BORDERLESS = 2,
	} wtype;
	int width;
	int height;
	int framerate;
	char *assetdir;
};

struct Window {
	SDL_Window *w;
	SDL_Renderer *r;
};

static const float SECOND = 1000.0f;

static const uint32_t wflags[] = {
	[WINDOWED] = SDL_WINDOW_SHOWN,
	[FULLSCREEN] = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN,
	[BORDERLESS] = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP,
};

static const char *const wdesc[] = {
	[WINDOWED] = "Windowed",
	[FULLSCREEN] = "Fullscreen",
	[BORDERLESS] = "Borderless Fullscreen",
};

static uint64_t pfreq = 0;

static struct Args args = {.cfgfile = "config.lua"};

static struct Config cfg = {
	.wtype = WINDOWED,
	.width = 1280,
	.height = 720,
	.framerate = 60,
	.assetdir = "./assets",
};

static inline uint64_t
now(void)
{
	return SDL_GetPerformanceCounter();
}

static void
logsdlerror(int category, char *file, int line)
{
	const char *err = SDL_GetError();
	if (strlen(err) != 0) {
		SDL_LogError(category, "%s:%d: %s", file, line, err);
	} else {
		SDL_LogError(category, "%s:%d", file, line);
	}
}

static void
parseargs(int argc, char *argv[], struct Args *args)
{
	for (int i = 0; i < argc;) {
		char *arg = argv[i++];
		if (strcmp(arg, "-c") == 0) {
			args->cfgfile = argv[i++];
		}
	}
}

static char *
allocpath(const char *a, const char *b)
{
	size_t n = (size_t)snprintf(NULL, 0, "%s/%s", a, b);
	char *ret = calloc(++n, sizeof(char)); /* incr for terminator */
	if (ret != NULL) {
		snprintf(ret, n, "%s/%s", a, b);
	}
	return ret;
}

static int
loadcfg(const char *f, struct Config *cfg)
{
	int rc = FAILURE;

	lua_State *state = luaL_newstate();
	if (state == NULL) {
		SDL_LogError(UNHANDLED, "%s: luaL_newstate failed", __func__);
		return rc;
	}

	luaL_openlibs(state);

	rc = luaL_loadfile(state, f) || lua_pcall(state, 0, 0, 0);
	if (rc != LUA_OK) {
		SDL_LogError(UNHANDLED, "%s: failed to load %s, %s", __func__,
			f, lua_tostring(state, -1));
		lua_pop(state, 1);
		rc = FAILURE;
		goto out;
	}

	lua_getglobal(state, "width");
	lua_getglobal(state, "height");
	if (!lua_isnumber(state, -2)) {
		SDL_LogError(UNHANDLED, "%s: width is not a number", __func__);
		rc = FAILURE;
		goto out;
	}
	if (!lua_isnumber(state, -1)) {
		SDL_LogError(UNHANDLED, "%s: height is not a number", __func__);
		rc = FAILURE;
		goto out;
	}
	cfg->width = (int)lua_tonumber(state, -2);
	cfg->height = (int)lua_tonumber(state, -1);

	rc = SUCCESS;
out:
	lua_close(state);
	return rc;
}

static float
calcframetime(int fps)
{
	assert(fps > 0);
	return SECOND / (float)fps;
}

static float
calcdelta(uint64_t begin, uint64_t end)
{
	assert(pfreq > 0);
	const float delta_ticks = (float)(end - begin);
	return (delta_ticks * SECOND) / (float)pfreq;
}

static void
delay(float frametime, uint64_t begin)
{
	if (calcdelta(begin, now()) < frametime) {
		const float delay = frametime - calcdelta(begin, now()) - 1.0f;
		if ((uint32_t)delay > 0) {
			SDL_Delay((uint32_t)delay);
		}
		while (calcdelta(begin, now()) < frametime) {
			/* wait */
		}
	}
}

static int
initwin(struct Config *cfg, const char *title, struct Window *win)
{
	SDL_LogInfo(UNHANDLED, "Window type: %s\n", wdesc[cfg->wtype]);
	win->w = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, cfg->width, cfg->height,
		wflags[cfg->wtype]);
	if (win->w == NULL) {
		logsdlerror(UNHANDLED, __FILE__, __LINE__);
		return FAILURE;
	}
	win->r = SDL_CreateRenderer(win->w, -1, SDL_RENDERER_ACCELERATED);
	if (win->r == NULL) {
		logsdlerror(UNHANDLED, __FILE__, __LINE__);
		return FAILURE;
	}
	SDL_SetRenderDrawColor(win->r, 0x00, 0x00, 0x00, 0xFF);
	return SUCCESS;
}

static void
finishwin(struct Window *win)
{
	if (win == NULL) {
		return;
	}
	if (win->r != NULL) {
		SDL_DestroyRenderer(win->r);
	}
	if (win->w != NULL) {
		SDL_DestroyWindow(win->w);
	}
}

static int
getrect(struct Window *win, SDL_Rect *rect)
{
	int w = 0;
	int h = 0;

	if (win == NULL || win->r == NULL) {
		return FAILURE;
	}
	int rc = SDL_GetRendererOutputSize(win->r, &w, &h);
	if (rc != SUCCESS) {
		logsdlerror(UNHANDLED, __FILE__, __LINE__);
		return rc;
	}
	rect->x = 0;
	rect->y = 0;
	rect->w = w;
	rect->h = h;
	return SUCCESS;
}

static void
keydown(SDL_KeyboardEvent *key, enum Loopstat *loopstat)
{
	switch (key->keysym.sym) {
	case SDLK_ESCAPE:
		*loopstat = STOP;
		break;
	default:
		break;
	}
}

static void
update(float delta_ms)
{
	(void)delta_ms;
}

int
main(int argc, char *argv[])
{
	int rc = FAILURE;
	enum Loopstat loopstat = RUN;
	struct Window win = {.w = NULL, .r = NULL};
	SDL_Rect winrect;
	SDL_Surface *s = NULL;
	SDL_Texture *t = NULL;
	SDL_Event ev;
	const char *const wintitle = "Hello, world!";
	const char *const testbmp = "test.bmp";
	uint64_t begin = 0;
	uint64_t end = 0;
	float delta = 0.0f;

	(void)argc;
	(void)argv;

	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

	parseargs(argc, argv, &args);

	loadcfg(args.cfgfile, &cfg);

	char *bmpfile = allocpath(cfg.assetdir, testbmp);
	if (bmpfile == NULL) {
		return EXIT_FAILURE;
	}

	rc = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if (rc != SUCCESS) {
		logsdlerror(UNHANDLED, __FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out0;
	}

	pfreq = SDL_GetPerformanceFrequency();

	rc = initwin(&cfg, wintitle, &win);
	if (rc != SUCCESS) {
		rc = EXIT_FAILURE;
		goto out1;
	}

	rc = getrect(&win, &winrect);
	if (rc != SUCCESS) {
		rc = EXIT_FAILURE;
		goto out2;
	}

	s = SDL_LoadBMP(bmpfile);
	if (s == NULL) {
		logsdlerror(UNHANDLED, __FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out2;
	}

	t = SDL_CreateTextureFromSurface(win.r, s);
	if (t == NULL) {
		logsdlerror(UNHANDLED, __FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out3;
	}
	SDL_FreeSurface(s);
	s = NULL;

	delta = calcframetime(cfg.framerate);
	begin = now();
	while (loopstat == RUN) {
		while (SDL_PollEvent(&ev) != 0) {
			switch (ev.type) {
			case SDL_QUIT:
				loopstat = STOP;
				break;
			case SDL_KEYDOWN:
				keydown(&ev.key, &loopstat);
				break;
			default:
				break;
			}
		}

		update(delta);

		/* render */
		{
			rc = SDL_RenderClear(win.r);
			if (rc != SUCCESS) {
				logsdlerror(UNHANDLED, __FILE__, __LINE__);
				rc = EXIT_FAILURE;
				goto out4;
			}

			rc = SDL_RenderCopy(win.r, t, NULL, &winrect);
			if (rc != SUCCESS) {
				logsdlerror(UNHANDLED, __FILE__, __LINE__);
				rc = EXIT_FAILURE;
				goto out4;
			}

			SDL_RenderPresent(win.r);
		}

		delay(delta, begin);

		end = now();
		delta = calcdelta(begin, end);
		begin = end;
	}

	rc = EXIT_SUCCESS;
out4:
	SDL_DestroyTexture(t);
out3:
	SDL_FreeSurface(s);
out2:
	finishwin(&win);
out1:
	SDL_Quit();
out0:
	free(bmpfile);
	return rc;
}
