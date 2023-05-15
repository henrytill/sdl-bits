#include <assert.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prelude.h"

enum {
  CENTERED = SDL_WINDOWPOS_CENTERED,
};

struct Args {
  char *configFile;
};

#define WINDOW_TYPE_VARIANTS                                               \
  X(WINDOWED, 0, SDL_WINDOW_SHOWN, "Windowed")                             \
  X(FULLSCREEN, 1, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN, "Fullscreen") \
  X(BORDERLESS, 2, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP, "Borderless Fullscreen")

enum WindowType {
#define X(variant, i, flags, str) variant = i,
  WINDOW_TYPE_VARIANTS
#undef X
};

struct Config {
  enum WindowType windowType;
  int x;
  int y;
  int width;
  int height;
  int frameRate;
  char *assetDir;
};

struct AudioState {
  const int sampleRate;
  const uint16_t bufferSize;
  const double frequency;
  const double maxVolume;
  double volume;
  uint64_t offset;
};

struct State {
  SDL_AudioDeviceID audioDevice;
  struct AudioState audio;
  int loopStat;
  int toneStat;
};

struct Window {
  SDL_Window *window;
  SDL_Renderer *renderer;
};

static const double second = 1000.0;

static const uint32_t winTypeFlags[] = {
#define X(variant, i, flags, str) [variant] = flags,
  WINDOW_TYPE_VARIANTS
#undef X
};

static const char *const winTypeStr[] = {
#define X(variant, i, flags, str) [variant] = str,
  WINDOW_TYPE_VARIANTS
#undef X
};

static uint64_t perfFreq = 0;

static struct Args args = {.configFile = "config.lua"};

static struct Config config = {
  .windowType = WINDOWED,
  .x = CENTERED,
  .y = CENTERED,
  .width = 1280,
  .height = 720,
  .frameRate = 60,
  .assetDir = "./assets",
};

static struct State state = {
  .audioDevice = 0,
  .audio = {
    .sampleRate = 48000,
    .bufferSize = 2048,
    .frequency = 440.0,
    .maxVolume = 0.25,
    .volume = 0.0,
    .offset = 0,
  },
  .loopStat = 1,
  .toneStat = 0,
};

///
/// Parse command line arguments and populate an Args struct with the results.
///
/// @param argc The number of arguments
/// @param argv The arguments
/// @param args The Args struct to populate
///
static void parseArgs(int argc, char *argv[], struct Args *args) {
  for (int i = 0; i < argc;) {
    char *arg = argv[i++];
    if (strcmp(arg, "-c") == 0)
      args->configFile = argv[i++];
  }
}

///
/// Join two paths together.  User is responsible for freeing the returned path.
///
/// @param a The first path
/// @param b The second path
/// @return A new path, or NULL on failure
///
static char *joinPath(const char *a, const char *b) {
  size_t len = (size_t)snprintf(NULL, 0, "%s/%s", a, b);
  char *ret = ecalloc(++len, sizeof(char)); // incr for terminator
  snprintf(ret, len, "%s/%s", a, b);
  return ret;
}

///
/// Load and parse a config file and populate a Config struct with the results.
///
/// @param file The config file to load
/// @param config The Config struct to populate
/// @return 0 on success, -1 on failure
///
static int loadConfig(const char *file, struct Config *config) {
  _cleanup_lua_State_ lua_State *state = luaL_newstate();
  if (state == NULL) {
    SDL_LogError(ERR, "%s: luaL_newstate failed", __func__);
    return -1;
  }
  luaL_openlibs(state);
  if (luaL_loadfile(state, file) || lua_pcall(state, 0, 0, 0) != LUA_OK) {
    SDL_LogError(ERR, "%s: failed to load %s, %s", __func__,
                 file, lua_tostring(state, -1));
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
  config->width = (int)lua_tonumber(state, -3);
  config->height = (int)lua_tonumber(state, -2);
  config->frameRate = (int)lua_tonumber(state, -1);
  return 0;
}

///
/// Calculate a sine wave and write it to the stream.
///
/// @param userData The userData passed to SDL_OpenAudioDevice
/// @param stream The stream to write to
/// @param len The length of the stream
///
static void calcSine(void *userData, uint8_t *stream, _unused_ int len) {
  struct AudioState *as = (struct AudioState *)userData;
  float *fstream = (float *)stream;

  assert((len / (4 * 2)) == as->bufferSize);
  const double sampleRate = (double)as->sampleRate;
  const uint64_t bufferSize = (uint64_t)as->bufferSize;

  for (uint64_t i = 0; i < bufferSize; ++i) {
    const double time = (double)((as->offset * bufferSize) + i) / sampleRate;
    const double x = 2.0 * M_PI * time * as->frequency;
    fstream[2 * i + 0] = (float)(as->volume * sin(x));
    fstream[2 * i + 1] = (float)(as->volume * sin(x));
  }
  as->offset += 1;
}

///
/// Calculate the time in milliseconds for a frame.
///
/// @param frameRate The frame rate
/// @return The time in milliseconds for a frame
///
static double calcFrameTime(const int frameRate) {
  extern const double second;
  assert((double)frameRate > 0);
  assert((double)frameRate < DBL_MAX);
  return second / (double)frameRate;
}

///
/// Calculate the time in milliseconds between two timestamps.
///
/// @param begin An initial timestamp in ticks
/// @param end A final timestamp in ticks
/// @return The time in milliseconds between the two timestamps
///
static double calcDelta(const uint64_t begin, const uint64_t end) {
  extern const double second;
  extern uint64_t perfFreq;
  assert(begin <= end);
  assert((double)perfFreq > 0);
  assert((double)perfFreq < DBL_MAX);
  const double deltaTicks = (double)(end - begin);
  return (deltaTicks * second) / (double)perfFreq;
}

///
/// Wait until the current frame end before returning.
///
/// @param frameTime The desired time in milliseconds for a frame
/// @param begin The timestamp in ticks when the frame started
///
static void delay(const double frameTime, const uint64_t begin) {
  if (calcDelta(begin, now()) >= frameTime) return;
  const uint32_t time = (uint32_t)(frameTime - calcDelta(begin, now()) - 1.0);
  if (time > 0) SDL_Delay(time);
  while (calcDelta(begin, now()) < frameTime) continue;
}

///
/// Create a window and renderer.
///
/// @param config The configuration.
/// @param title The window title.
/// @param win The window to initialize.
/// @return 0 on success, -1 on failure.
///
static int initWindow(struct Config *config, const char *title, struct Window *win) {
  extern const uint32_t winTypeFlags[];
  extern const char *const winTypeStr[];

  SDL_LogInfo(APP, "Window type: %s", winTypeStr[config->windowType]);
  win->window = SDL_CreateWindow(title,
                                 config->x, config->y,
                                 config->width, config->height,
                                 winTypeFlags[config->windowType]);
  if (win->window == NULL) {
    sdl_error("SDL_CreateWindow failed");
    return -1;
  }
  win->renderer = SDL_CreateRenderer(win->window, -1, SDL_RENDERER_ACCELERATED);
  if (win->renderer == NULL) {
    SDL_DestroyWindow(win->window);
    sdl_error("SDL_CreateRenderer failed");
    return -1;
  }
  return SDL_SetRenderDrawColor(win->renderer, 0x00, 0x00, 0x00, 0xFF);
}

///
/// Destroy a window and renderer.
///
/// @param win The window to destroy.
///
static void finishWindow(struct Window *win) {
  if (win == NULL) return;
  if (win->renderer != NULL) SDL_DestroyRenderer(win->renderer);
  if (win->window != NULL) SDL_DestroyWindow(win->window);
}

///
/// Create a window and renderer.
///
/// @param config The configuration.
/// @param title The window title.
/// @return The window on success, NULL on failure.
///
static struct Window *createWindow(struct Config *config, const char *title) {
  struct Window *win = emalloc(sizeof(struct Window));
  if (initWindow(config, title, win) != 0) {
    free(win);
    return NULL;
  }
  return win;
}

///
/// Destroy a window and renderer.
///
/// @param win The win to destroy.
///
static void destroyWindow(struct Window *win) {
  if (win == NULL) return;
  finishWindow(win);
  free(win);
}

DEFINE_TRIVIAL_CLEANUP_FUNC(struct Window *, destroyWindow)
#define _cleanup_Window_ _cleanup_(destroyWindowp)

///
/// Get the window's rectangle.
///
/// @param win The window.
/// @param rect The rectangle to initialize.
/// @return 0 on success, -1 on failure.
///
static int getRect(struct Window *win, SDL_Rect *rect) {
  if (win == NULL || win->renderer == NULL)
    return -1;
  const int rc = SDL_GetRendererOutputSize(win->renderer, &rect->w, &rect->h);
  if (rc != 0) {
    sdl_error("SDL_GetRendererOutputSize failed");
    return -1;
  }
  return 0;
}

///
/// Handle keydown events.
///
/// @param key The keydown event.
/// @param state The state.
///
static void handleKeydown(SDL_KeyboardEvent *key, struct State *state) {
  switch (key->keysym.sym) {
  case SDLK_ESCAPE:
    state->loopStat = 0;
    break;
  case SDLK_F1:
    state->toneStat = (state->toneStat == 1) ? 0 : 1;
    SDL_LockAudioDevice(state->audioDevice);
    state->audio.volume = state->toneStat * state->audio.maxVolume;
    state->audio.offset = 0;
    SDL_UnlockAudioDevice(state->audioDevice);
    break;
  }
}

static void update(double delta) {
  (void)delta;
}

int main(int argc, char *argv[]) {
  extern uint64_t perfFreq;
  extern struct Args args;
  extern struct Config config;
  extern struct State state;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
  parseArgs(argc, argv, &args);
  loadConfig(args.configFile, &config);

  int rc = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  if (rc != 0) {
    sdl_error("init failed");
    return EXIT_FAILURE;
  }

  AT_EXIT(SDL_Quit);

  perfFreq = SDL_GetPerformanceFrequency();

  SDL_AudioSpec want = {
    .freq = state.audio.sampleRate,
    .format = AUDIO_F32,
    .channels = 2,
    .samples = state.audio.bufferSize,
    .callback = calcSine,
    .userdata = (void *)&state.audio,
  };
  SDL_AudioSpec have = {0};

  _cleanup_SDL_AudioDeviceID_ SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  state.audioDevice = audioDevice;
  if (state.audioDevice < 2) {
    sdl_error("SDL_OpenAudio failed");
    return EXIT_FAILURE;
  }

  const char *const winTitle = "Hello, world!";
  _cleanup_Window_ struct Window *win = createWindow(&config, winTitle);
  if (win == NULL)
    return EXIT_FAILURE;

  SDL_Rect winRect = {0, 0, 0, 0};
  rc = getRect(win, &winRect);
  if (rc != 0)
    return EXIT_FAILURE;

  _cleanup_SDL_Texture_ SDL_Texture *texture = ({
    const char *const testBmp = "test.bmp";
    _cleanup_str_ char *bmpFile = joinPath(config.assetDir, testBmp);
    if (bmpFile == NULL)
      return EXIT_FAILURE;

    _cleanup_SDL_Surface_ SDL_Surface *surface = SDL_LoadBMP(bmpFile);
    if (surface == NULL) {
      sdl_error("SDL_LoadBMP failed");
      return EXIT_FAILURE;
    }

    SDL_Texture *tmp = SDL_CreateTextureFromSurface(win->renderer, surface);
    if (tmp == NULL) {
      sdl_error("SDL_CreateTextureFromSurface failed");
      return EXIT_FAILURE;
    }
    tmp;
  });

  const double frameTime = calcFrameTime(config.frameRate);

  SDL_PauseAudioDevice(state.audioDevice, 0);

  SDL_Event event = {0};
  double delta = frameTime;
  uint64_t begin = now();
  uint64_t end = 0;

  while (state.loopStat == 1) {
    while (SDL_PollEvent(&event) != 0) {
      switch (event.type) {
      case SDL_QUIT:
        state.loopStat = 0;
        break;
      case SDL_KEYDOWN:
        handleKeydown(&event.key, &state);
        break;
      }
    }

    update(delta);

    rc = SDL_RenderClear(win->renderer);
    if (rc != 0) {
      sdl_error("SDL_RenderClear failed");
      return EXIT_FAILURE;
    }
    rc = SDL_RenderCopy(win->renderer, texture, NULL, &winRect);
    if (rc != 0) {
      sdl_error("SDL_RenderCopy failed");
      return EXIT_FAILURE;
    }
    SDL_RenderPresent(win->renderer);

    delay(frameTime, begin);
    end = now();
    delta = calcDelta(begin, end);
    begin = end;
  }

  SDL_PauseAudioDevice(state.audioDevice, 1);

  return EXIT_SUCCESS;
}
