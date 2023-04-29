#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <SDL_audio.h>

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

struct AudioState {
  const int samplerate;
  const uint16_t buffsize;
  const double volume;
  const double frequency;
  uint64_t offset;
};

struct Window {
  SDL_Window *window;
  SDL_Renderer *renderer;
};

static const float SECOND = 1000.0f;

static const uint32_t wtypeflags[] = {
  [WINDOWED] = SDL_WINDOW_SHOWN,
  [FULLSCREEN] = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN,
  [BORDERLESS] = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP,
};

static const char *const wtypestr[] = {
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

static struct AudioState as = {
  .samplerate = 48000,
  .buffsize = 2048,
  .volume = 0.25,
  .frequency = 440.0,
  .offset = 0,
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

void calcsine(void *userdata, uint8_t *stream, int len) {
  struct AudioState *as = (struct AudioState *)userdata;
  float *fstream = (float *)stream;

  assert((len / 8) == as->buffsize);

  const double samplerate = (double)as->samplerate;
  const uint64_t buffsize = (uint64_t)as->buffsize;

  for (uint64_t i = 0; i < buffsize; ++i) {
    const double time = (double)((as->offset * buffsize) + i) / samplerate;
    const double x = 2.0 * M_PI * time * as->frequency;
    fstream[2 * i + 0] = (float)(as->volume * sin(x));
    fstream[2 * i + 1] = (float)(as->volume * sin(x));
  }

  as->offset += 1;
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
  SDL_LogInfo(APP, "Window type: %s", wtypestr[cfg->wtype]);
  win->window = SDL_CreateWindow(title,
                                 cfg->x, cfg->y,
                                 cfg->width, cfg->height,
                                 wtypeflags[cfg->wtype]);
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
  SDL_AudioSpec want, have;
  SDL_AudioDeviceID devid;
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

  rc = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  if (rc != SUCCESS) {
    logsdlerr("SDL_Init failed");
    return EXIT_FAILURE;
  }

  pfreq = SDL_GetPerformanceFrequency();

  want.freq = as.samplerate;
  want.format = AUDIO_F32;
  want.channels = 2;
  want.samples = as.buffsize;
  want.callback = calcsine;
  want.userdata = (void *)&as;

  devid = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if (devid < 2) {
    logsdlerr("SDL_OpenAudio failed");
    goto out0;
  }

  rc = initwin(&dcfg, wintitle, &win);
  if (rc != SUCCESS)
    goto out1;

  rc = getrect(&win, &winrect);
  if (rc != SUCCESS)
    goto out2;

  bmpfile = allocpath(dcfg.assetdir, testbmp);
  if (bmpfile == NULL)
    goto out2;

  surface = SDL_LoadBMP(bmpfile);
  if (surface == NULL) {
    logsdlerr("SDL_LoadBMP failed");
    goto out3;
  }

  texture = SDL_CreateTextureFromSurface(win.renderer, surface);
  if (texture == NULL) {
    logsdlerr("SDL_CreateTextureFromSurface failed");
    goto out4;
  }
  SDL_FreeSurface(surface);
  surface = NULL;

  SDL_PauseAudioDevice(devid, 0);

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
      goto out5;
    }
    rc = SDL_RenderCopy(win.renderer, texture, NULL, &winrect);
    if (rc != SUCCESS) {
      logsdlerr("SDL_RenderCopy failed");
      goto out5;
    }
    SDL_RenderPresent(win.renderer);

    delay(delta, begin);
    end = now();
    delta = calcdelta(begin, end);
    begin = end;
  }

  SDL_PauseAudioDevice(devid, 1);

  ret = EXIT_SUCCESS;
out5:
  SDL_DestroyTexture(texture);
out4:
  SDL_FreeSurface(surface);
out3:
  free(bmpfile);
out2:
  finishwin(&win);
out1:
  SDL_CloseAudioDevice(devid);
out0:
  SDL_Quit();
  return ret;
}
