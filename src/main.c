#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#define now SDL_GetPerformanceCounter

enum {
  SUCCESS = 0,
  FAILURE = 1,
};

enum {
  APP = SDL_LOG_CATEGORY_CUSTOM,
  ERR,
};

enum Loopstat {
  STOP = 0,
  RUN = 1,
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
  int x;
  int y;
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

static struct Args dargs = {.cfgfile = "config.lua"};

static struct Config dcfg = {
  .wtype = WINDOWED,
  .x = SDL_WINDOWPOS_CENTERED,
  .y = SDL_WINDOWPOS_CENTERED,
  .width = 1280,
  .height = 720,
  .framerate = 60,
  .assetdir = "./assets",
};

static void logsdlerr(char *msg) {
  const char *err = SDL_GetError();
  if (strlen(err) != 0)
    SDL_LogError(ERR, "%s (%s)", msg, err);
  else
    SDL_LogError(ERR, "%s", msg);
}

static void parseargs(int argc, char *argv[], struct Args *args) {
  for (int i = 0; i < argc;) {
    char *arg = argv[i++];
    if (strcmp(arg, "-c") == 0)
      args->cfgfile = argv[i++];
  }
}

static char *allocpath(const char *a, const char *b) {
  size_t n = (size_t)snprintf(NULL, 0, "%s/%s", a, b);
  char *ret = calloc(++n, sizeof(char)); /* incr for terminator */
  if (ret != NULL) snprintf(ret, n, "%s/%s", a, b);
  return ret;
}

static int loadcfg(const char *f, struct Config *cfg) {
  int ret = FAILURE;

  lua_State *state = luaL_newstate();
  if (state == NULL) {
    SDL_LogError(ERR, "%s: luaL_newstate failed", __func__);
    return ret;
  }
  luaL_openlibs(state);
  if (luaL_loadfile(state, f) || lua_pcall(state, 0, 0, 0) != LUA_OK) {
    SDL_LogError(ERR, "%s: failed to load %s, %s", __func__, f, lua_tostring(state, -1));
    goto out;
  }
  lua_getglobal(state, "width");
  lua_getglobal(state, "height");
  if (!lua_isnumber(state, -2)) {
    SDL_LogError(ERR, "%s: width is not a number", __func__);
    goto out;
  }
  if (!lua_isnumber(state, -1)) {
    SDL_LogError(ERR, "%s: height is not a number", __func__);
    goto out;
  }
  cfg->width = (int)lua_tonumber(state, -2);
  cfg->height = (int)lua_tonumber(state, -1);

  ret = SUCCESS;
out:
  lua_close(state);
  return ret;
}

static float calcframetime(int fps) {
  assert(fps > 0);
  return SECOND / (float)fps;
}

static float calcdelta(uint64_t begin, uint64_t end) {
  assert(pfreq > 0);
  const float delta_ticks = (float)(end - begin);
  return (delta_ticks * SECOND) / (float)pfreq;
}

static void delay(float frametime, uint64_t begin) {
  if (calcdelta(begin, now()) >= frametime) return;
  const uint32_t delay = (uint32_t)(frametime - calcdelta(begin, now()) - 1.0f);
  if (delay > 0) SDL_Delay(delay);
  while (calcdelta(begin, now()) < frametime) continue;
}

static int initwin(struct Config *cfg, const char *title, struct Window *win) {
  SDL_LogInfo(APP, "Window type: %s\n", wdesc[cfg->wtype]);
  if ((win->w = SDL_CreateWindow(title, cfg->x, cfg->y, cfg->width, cfg->height, wflags[cfg->wtype])) == NULL) {
    logsdlerr("SDL_CreateWindow failed");
    return FAILURE;
  }
  if ((win->r = SDL_CreateRenderer(win->w, -1, SDL_RENDERER_ACCELERATED)) == NULL) {
    logsdlerr("SDL_CreateRenderer failed");
    return FAILURE;
  }
  SDL_SetRenderDrawColor(win->r, 0x00, 0x00, 0x00, 0xFF);
  return SUCCESS;
}

static void finishwin(struct Window *win) {
  if (win == NULL) return;
  if (win->r != NULL) SDL_DestroyRenderer(win->r);
  if (win->w != NULL) SDL_DestroyWindow(win->w);
}

static int getrect(struct Window *win, SDL_Rect *rect) {
  if (win == NULL || win->r == NULL)
    return FAILURE;
  if (SDL_GetRendererOutputSize(win->r, &rect->w, &rect->h) != SUCCESS) {
    logsdlerr("SDL_GetRendererOutputSize failed");
    return FAILURE;
  }
  return SUCCESS;
}

static void keydown(SDL_KeyboardEvent *key, enum Loopstat *loopstat) {
  switch (key->keysym.sym) {
  case SDLK_ESCAPE:
    *loopstat = STOP;
    break;
  }
}

static void update(float delta) {
  (void)delta;
}

int main(int argc, char *argv[]) {
  int ret = EXIT_FAILURE;
  enum Loopstat loopstat = RUN;
  struct Window win = {NULL, NULL};
  SDL_Rect winrect = {0, 0, 0, 0};
  SDL_Surface *s = NULL;
  SDL_Texture *t = NULL;
  SDL_Event ev;
  const char *const wintitle = "Hello, world!";
  const char *const testbmp = "test.bmp";
  char *bmpfile;
  uint64_t begin, end;
  float delta;

  (void)argc;
  (void)argv;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

  parseargs(argc, argv, &dargs);

  loadcfg(dargs.cfgfile, &dcfg);

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != SUCCESS) {
    logsdlerr("SDL_Init failed");
    return EXIT_FAILURE;
  }

  pfreq = SDL_GetPerformanceFrequency();

  if (initwin(&dcfg, wintitle, &win) != SUCCESS)
    goto out0;

  if (getrect(&win, &winrect) != SUCCESS)
    goto out1;

  if ((bmpfile = allocpath(dcfg.assetdir, testbmp)) == NULL)
    goto out1;

  if ((s = SDL_LoadBMP(bmpfile)) == NULL) {
    logsdlerr("SDL_LoadBMP failed");
    goto out2;
  }

  if ((t = SDL_CreateTextureFromSurface(win.r, s)) == NULL) {
    logsdlerr("SDL_CreateTextureFromSurface failed");
    goto out3;
  }
  SDL_FreeSurface(s);
  s = NULL;

  delta = calcframetime(dcfg.framerate);
  begin = now();
  while (loopstat == RUN) {
    while (SDL_PollEvent(&ev) != 0)
      switch (ev.type) {
      case SDL_QUIT:
        loopstat = STOP;
        break;
      case SDL_KEYDOWN:
        keydown(&ev.key, &loopstat);
        break;
      }

    update(delta);

    if (SDL_RenderClear(win.r) != SUCCESS) {
      logsdlerr("SDL_RenderClear failed");
      goto out4;
    }
    if (SDL_RenderCopy(win.r, t, NULL, &winrect) != SUCCESS) {
      logsdlerr("SDL_RenderCopy failed");
      goto out4;
    }
    SDL_RenderPresent(win.r);

    delay(delta, begin);
    end = now();
    delta = calcdelta(begin, end);
    begin = end;
  }

  ret = EXIT_SUCCESS;
out4:
  SDL_DestroyTexture(t);
out3:
  SDL_FreeSurface(s);
out2:
  free(bmpfile);
out1:
  finishwin(&win);
out0:
  SDL_Quit();
  return ret;
}
