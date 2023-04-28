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

/* https://github.com/libsdl-org/SDL/commit/c265fb74b045fcaf6310f116c212d27c3e1104e9 */
#ifndef SDL_HINT_VIDEO_DRIVER
#define SDL_HINT_VIDEO_DRIVER SDL_HINT_VIDEODRIVER
#endif

#define now SDL_GetPerformanceCounter

enum {
  SUCCESS = 0,
  FAILURE = 1,
};

enum {
  APP = SDL_LOG_CATEGORY_CUSTOM,
  ERR,
};

enum {
  CENTERED = SDL_WINDOWPOS_CENTERED,
};

enum Loopstat {
  STOP = 0,
  RUN = 1,
};

struct Args {
  char *cfgfile;
};

struct Config {
  enum WType {
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
  SDL_Window *window;
  SDL_Renderer *renderer;
};

static const float SECOND = 1000.0f;

static const char *const VIDEODRIVER = "wayland,x11";

static const uint32_t WFLAGS[] = {
  [WINDOWED] = SDL_WINDOW_SHOWN,
  [FULLSCREEN] = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN,
  [BORDERLESS] = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP,
};

static const char *const WDESC[] = {
  [WINDOWED] = "Windowed",
  [FULLSCREEN] = "Fullscreen",
  [BORDERLESS] = "Borderless Fullscreen",
};

static uint64_t pfreq = 0;

static struct Args dargs = {.cfgfile = "config.lua"};

static struct Config dcfg = {
  .wtype = WINDOWED,
  .x = CENTERED,
  .y = CENTERED,
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
    SDL_LogError(ERR, "%s: failed to load %s, %s", __func__,
                 f, lua_tostring(state, -1));
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
  SDL_LogInfo(APP, "Window type: %s", WDESC[cfg->wtype]);
  win->window = SDL_CreateWindow(title,
                                 cfg->x, cfg->y,
                                 cfg->width, cfg->height,
                                 WFLAGS[cfg->wtype]);
  if (win->window == NULL) {
    logsdlerr("SDL_CreateWindow failed");
    return FAILURE;
  }
  win->renderer = SDL_CreateRenderer(win->window, -1, SDL_RENDERER_ACCELERATED);
  if (win->renderer == NULL) {
    logsdlerr("SDL_CreateRenderer failed");
    return FAILURE;
  }
  SDL_SetRenderDrawColor(win->renderer, 0x00, 0x00, 0x00, 0xFF);
  return SUCCESS;
}

static void finishwin(struct Window *win) {
  if (win == NULL) return;
  if (win->renderer != NULL) SDL_DestroyRenderer(win->renderer);
  if (win->window != NULL) SDL_DestroyWindow(win->window);
}

static int getrect(struct Window *win, SDL_Rect *rect) {
  if (win == NULL || win->renderer == NULL)
    return FAILURE;
  const int rc = SDL_GetRendererOutputSize(win->renderer, &rect->w, &rect->h);
  if (rc != SUCCESS) {
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
  int rc;
  enum Loopstat loopstat = RUN;
  struct Window win = {NULL, NULL};
  SDL_Rect winrect = {0, 0, 0, 0};
  SDL_Surface *surface = NULL;
  SDL_Texture *texture = NULL;
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

  rc = SDL_SetHint(SDL_HINT_VIDEO_DRIVER, VIDEODRIVER);
  if (rc != SDL_TRUE) {
    logsdlerr("SDL_SetHint failed");
    return EXIT_FAILURE;
  }

  rc = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  if (rc != SUCCESS) {
    logsdlerr("SDL_Init failed");
    return EXIT_FAILURE;
  }

  pfreq = SDL_GetPerformanceFrequency();

  rc = initwin(&dcfg, wintitle, &win);
  if (rc != SUCCESS)
    goto out0;

  rc = getrect(&win, &winrect);
  if (rc != SUCCESS)
    goto out1;

  bmpfile = allocpath(dcfg.assetdir, testbmp);
  if (bmpfile == NULL)
    goto out1;

  surface = SDL_LoadBMP(bmpfile);
  if (surface == NULL) {
    logsdlerr("SDL_LoadBMP failed");
    goto out2;
  }

  texture = SDL_CreateTextureFromSurface(win.renderer, surface);
  if (texture == NULL) {
    logsdlerr("SDL_CreateTextureFromSurface failed");
    goto out3;
  }
  SDL_FreeSurface(surface);
  surface = NULL;

  delta = calcframetime(dcfg.framerate);
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
      }
    }

    update(delta);

    rc = SDL_RenderClear(win.renderer);
    if (rc != SUCCESS) {
      logsdlerr("SDL_RenderClear failed");
      goto out4;
    }
    rc = SDL_RenderCopy(win.renderer, texture, NULL, &winrect);
    if (rc != SUCCESS) {
      logsdlerr("SDL_RenderCopy failed");
      goto out4;
    }
    SDL_RenderPresent(win.renderer);

    delay(delta, begin);
    end = now();
    delta = calcdelta(begin, end);
    begin = end;
  }

  ret = EXIT_SUCCESS;
out4:
  SDL_DestroyTexture(texture);
out3:
  SDL_FreeSurface(surface);
out2:
  free(bmpfile);
out1:
  finishwin(&win);
out0:
  SDL_Quit();
  return ret;
}
