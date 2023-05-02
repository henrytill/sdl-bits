#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prelude.h"

#define now SDL_GetPerformanceCounter

enum {
  APP = SDL_LOG_CATEGORY_CUSTOM,
  ERR,
};

enum {
  CENTERED = SDL_WINDOWPOS_CENTERED,
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
  const double frequency;
  const double maxvolume;
  double volume;
  uint64_t offset;
};

struct State {
  SDL_AudioDeviceID audiodev;
  struct AudioState audio;
  int loopstat;
  int tonestat;
};

struct Window {
  SDL_Window *window;
  SDL_Renderer *renderer;
};

static const double second = 1000.0;

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

static struct Args args = {.cfgfile = "config.lua"};

static struct Config config = {
  .wtype = WINDOWED,
  .x = CENTERED,
  .y = CENTERED,
  .width = 1280,
  .height = 720,
  .framerate = 60,
  .assetdir = "./assets",
};

static struct State state = {
  .audiodev = 0,
  .audio = {
    .samplerate = 48000,
    .buffsize = 2048,
    .frequency = 440.0,
    .maxvolume = 0.25,
    .volume = 0.0,
    .offset = 0,
  },
  .loopstat = 1,
  .tonestat = 0,
};

/**
 * Log a msg and the contents of SDL_GetError().
 *
 * @param msg The message to log
 */
static void logsdlerr(char *msg) {
  const char *err = SDL_GetError();
  if (strlen(err) != 0)
    SDL_LogError(ERR, "%s (%s)", msg, err);
  else
    SDL_LogError(ERR, "%s", msg);
}

/**
 * Parse command line arguments and populate an Args struct with the results.
 *
 * @param argc The number of arguments
 * @param argv The arguments
 * @param args The Args struct to populate
 */
static void parseargs(int argc, char *argv[], struct Args *args) {
  for (int i = 0; i < argc;) {
    char *arg = argv[i++];
    if (strcmp(arg, "-c") == 0)
      args->cfgfile = argv[i++];
  }
}

/**
 * Join two paths together.  User is responsible for freeing the returned path.
 *
 * @param a The first path
 * @param b The second path
 * @return A new path, or NULL on failure
 */
static char *joinpath(const char *a, const char *b) {
  size_t n = (size_t)snprintf(NULL, 0, "%s/%s", a, b);
  char *ret = calloc(++n, sizeof(char)); /* incr for terminator */
  if (ret != NULL) snprintf(ret, n, "%s/%s", a, b);
  return ret;
}

/**
 * Load and parse a config file and populate a Config struct with the results.
 *
 * @param f The config file to load
 * @param cfg The Config struct to populate
 * @return 0 on success, -1 on failure
 */
static int loadcfg(const char *f, struct Config *cfg) {
  _cleanup_lua_State_ lua_State *state = luaL_newstate();
  if (state == NULL) {
    SDL_LogError(ERR, "%s: luaL_newstate failed", __func__);
    return -1;
  }
  luaL_openlibs(state);
  if (luaL_loadfile(state, f) || lua_pcall(state, 0, 0, 0) != LUA_OK) {
    SDL_LogError(ERR, "%s: failed to load %s, %s", __func__,
                 f, lua_tostring(state, -1));
    return -1;
  }
  lua_getglobal(state, "width");
  lua_getglobal(state, "height");
  lua_getglobal(state, "framerate");
  if (!lua_isnumber(state, -3)) {
    SDL_LogError(ERR, "%s: width is not a number", __func__);
    return -1;
  }
  if (!lua_isnumber(state, -2)) {
    SDL_LogError(ERR, "%s: height is not a number", __func__);
    return -1;
  }
  if (!lua_isnumber(state, -1)) {
    SDL_LogError(ERR, "%s: framerate is not a number", __func__);
    return -1;
  }
  cfg->width = (int)lua_tonumber(state, -3);
  cfg->height = (int)lua_tonumber(state, -2);
  cfg->framerate = (int)lua_tonumber(state, -1);
  return 0;
}

/**
 * Calculate a sine wave and write it to the stream.
 *
 * @param userdata The userdata passed to SDL_OpenAudioDevice
 * @param stream The stream to write to
 * @param len The length of the stream
 */
static void calcsine(void *userdata, uint8_t *stream, __attribute__((unused)) int len) {
  struct AudioState *as = (struct AudioState *)userdata;
  float *fstream = (float *)stream;

  assert((len / (4 * 2)) == as->buffsize);
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

/**
 * Calculate the time in milliseconds for a frame.
 *
 * @param fps The framerate
 * @return The time in milliseconds for a frame
 */
static double calcframetime(int fps) {
  extern const double second;
  assert(fps > 0);
  return second / (double)fps;
}

/**
 * Calculate the time in milliseconds between two timestamps.
 *
 * @param begin An initial timestamp in ticks
 * @param end A final timestamp in ticks
 * @return The time in milliseconds between the two timestamps
 */
static double calcdelta(uint64_t begin, uint64_t end) {
  extern const double second;
  extern uint64_t pfreq;
  assert(pfreq > 0);
  assert(begin <= end);
  const double delta_ticks = (double)(end - begin);
  return (delta_ticks * second) / (double)pfreq;
}

/**
 * Wait until the current frame end before returning.
 *
 * @param frametime The desired time in milliseconds for a frame
 * @param begin The timestamp in ticks when the frame started
 */
static void delay(double frametime, uint64_t begin) {
  if (calcdelta(begin, now()) >= frametime) return;
  const uint32_t delay = (uint32_t)(frametime - calcdelta(begin, now()) - 1.0);
  if (delay > 0) SDL_Delay(delay);
  while (calcdelta(begin, now()) < frametime) continue;
}

/**
 * Create a window and renderer.
 *
 * @param cfg The configuration.
 * @param title The window title.
 * @param win The window to initialize.
 * @return 0 on success, -1 on failure.
 */
static int initwin(struct Config *cfg, const char *title, struct Window *win) {
  extern const uint32_t wtypeflags[];
  extern const char *const wtypestr[];
  SDL_LogInfo(APP, "Window type: %s", wtypestr[cfg->wtype]);
  win->window = SDL_CreateWindow(title,
                                 cfg->x, cfg->y,
                                 cfg->width, cfg->height,
                                 wtypeflags[cfg->wtype]);
  if (win->window == NULL) {
    logsdlerr("SDL_CreateWindow failed");
    return -1;
  }
  win->renderer = SDL_CreateRenderer(win->window, -1, SDL_RENDERER_ACCELERATED);
  if (win->renderer == NULL) {
    SDL_DestroyWindow(win->window);
    logsdlerr("SDL_CreateRenderer failed");
    return -1;
  }
  SDL_SetRenderDrawColor(win->renderer, 0x00, 0x00, 0x00, 0xFF);
  return 0;
}

/**
 * Destroy a window and renderer.
 *
 * @param win The window to destroy.
 */
static void finishwin(struct Window *win) {
  if (win == NULL) return;
  if (win->renderer != NULL) SDL_DestroyRenderer(win->renderer);
  if (win->window != NULL) SDL_DestroyWindow(win->window);
}

/**
 * Create a window and renderer.
 *
 * @param cfg The configuration.
 * @param title The window title.
 * @return The window on success, NULL on failure.
 */
static struct Window *createwin(struct Config *cfg, const char *title) {
  struct Window *win = malloc(sizeof(struct Window));
  if (win == NULL) {
    SDL_LogError(ERR, "%s: malloc failed", __func__);
    return NULL;
  }
  if (initwin(cfg, title, win) != 0) {
    free(win);
    return NULL;
  }
  return win;
}

/**
 * Destroy a window and renderer.
 *
 * @param win The window to destroy.
 */
static void destroywin(struct Window *win) {
  if (win == NULL) return;
  finishwin(win);
  free(win);
}

DEFINE_TRIVIAL_CLEANUP_FUNC(struct Window *, destroywin);
#define _cleanup_Window_ _cleanup_(destroywinp)

/**
 * Get the window's rectangle.
 *
 * @param win The window.
 * @param rect The rectangle to initialize.
 * @return 0 on success, -1 on failure.
 */
static int getrect(struct Window *win, SDL_Rect *rect) {
  if (win == NULL || win->renderer == NULL)
    return -1;
  const int rc = SDL_GetRendererOutputSize(win->renderer, &rect->w, &rect->h);
  if (rc != 0) {
    logsdlerr("SDL_GetRendererOutputSize failed");
    return -1;
  }
  return 0;
}

/**
 * Handle keydown events.
 *
 * @param key The keydown event.
 * @param state The state.
 */
static void keydown(SDL_KeyboardEvent *key, struct State *state) {
  switch (key->keysym.sym) {
  case SDLK_ESCAPE:
    state->loopstat = 0;
    break;
  case SDLK_F1:
    state->tonestat = (state->tonestat == 1) ? 0 : 1;
    SDL_LockAudioDevice(state->audiodev);
    state->audio.volume = state->tonestat * state->audio.maxvolume;
    state->audio.offset = 0;
    SDL_UnlockAudioDevice(state->audiodev);
    break;
  }
}

static void update(double delta) {
  (void)delta;
}

int main(int argc, char *argv[]) {
  extern uint64_t pfreq;
  extern struct Args args;
  extern struct Config config;
  extern struct State state;

  int rc;
  SDL_AudioSpec want, have;
  SDL_Rect winrect = {0, 0, 0, 0};
  SDL_Event ev;
  const char *const wintitle = "Hello, world!";
  const char *const testbmp = "test.bmp";
  uint64_t begin, end;
  double delta;

  (void)argc;
  (void)argv;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
  parseargs(argc, argv, &args);
  loadcfg(args.cfgfile, &config);

  rc = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  if (rc != 0) {
    logsdlerr("init failed");
    return EXIT_FAILURE;
  }

  ATEXIT(SDL_Quit);

  pfreq = SDL_GetPerformanceFrequency();

  want.freq = state.audio.samplerate;
  want.format = AUDIO_F32;
  want.channels = 2;
  want.samples = state.audio.buffsize;
  want.callback = calcsine;
  want.userdata = (void *)&state.audio;

  _cleanup_SDL_AudioDeviceID_ SDL_AudioDeviceID devid = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  state.audiodev = devid;
  if (state.audiodev < 2) {
    logsdlerr("SDL_OpenAudio failed");
    return EXIT_FAILURE;
  }

  _cleanup_Window_ struct Window *win = createwin(&config, wintitle);
  if (win == NULL)
    return EXIT_FAILURE;

  rc = getrect(win, &winrect);
  if (rc != 0)
    return EXIT_FAILURE;

  _cleanup_str_ char *bmpfile = joinpath(config.assetdir, testbmp);
  if (bmpfile == NULL)
    return EXIT_FAILURE;

  _cleanup_SDL_Surface_ SDL_Surface *surface = SDL_LoadBMP(bmpfile);
  if (surface == NULL) {
    logsdlerr("SDL_LoadBMP failed");
    return EXIT_FAILURE;
  }

  _cleanup_SDL_Texture_ SDL_Texture *texture = SDL_CreateTextureFromSurface(win->renderer, surface);
  if (texture == NULL) {
    logsdlerr("SDL_CreateTextureFromSurface failed");
    return EXIT_FAILURE;
  }

  SDL_FreeSurface(surface);
  assert(surface != NULL);
  surface = NULL;

  freestr(bmpfile);
  assert(bmpfile != NULL);
  bmpfile = NULL;

  const double frametime = calcframetime(config.framerate);

  SDL_PauseAudioDevice(state.audiodev, 0);

  delta = frametime;
  begin = now();
  while (state.loopstat == 1) {
    while (SDL_PollEvent(&ev) != 0) {
      switch (ev.type) {
      case SDL_QUIT:
        state.loopstat = 0;
        break;
      case SDL_KEYDOWN:
        keydown(&ev.key, &state);
        break;
      }
    }

    update(delta);

    rc = SDL_RenderClear(win->renderer);
    if (rc != 0) {
      logsdlerr("SDL_RenderClear failed");
      return EXIT_FAILURE;
    }
    rc = SDL_RenderCopy(win->renderer, texture, NULL, &winrect);
    if (rc != 0) {
      logsdlerr("SDL_RenderCopy failed");
      return EXIT_FAILURE;
    }
    SDL_RenderPresent(win->renderer);

    delay(frametime, begin);
    end = now();
    delta = calcdelta(begin, end);
    begin = end;
  }

  SDL_PauseAudioDevice(state.audiodev, 1);

  return EXIT_SUCCESS;
}
