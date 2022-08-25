#include <SDL.h>

enum {
  SUCCESS = 0,
  FAILURE = 1,
};

static int enumdisp(void) {
  SDL_DisplayMode mode;

  int ndisp = SDL_GetNumVideoDisplays();
  if (ndisp < 0) {
    const char *err = SDL_GetError();
    SDL_Log("Failed to get number of video displays: %s", err);
    return FAILURE;
  }
  for (int i = 0; i < ndisp; ++i) {
    if (SDL_GetCurrentDisplayMode(i, &mode) != SUCCESS) {
      const char *err = SDL_GetError();
      SDL_Log("Failed to get mode for display #%d: %s", i, err);
      return FAILURE;
    }
    SDL_Log("Display #%d: mode is %dx%d @ %d hz.", i, mode.w, mode.h, mode.refresh_rate);
  }
  return SUCCESS;
}

int main(int argc, char *argv[]) {
  int ret = EXIT_FAILURE;

  (void)argc;
  (void)argv;

  if (SDL_Init(SDL_INIT_VIDEO) != SUCCESS)
    return ret;
  if (enumdisp() != SUCCESS)
    goto out;
  ret = EXIT_SUCCESS;
out:
  SDL_Quit();
  return ret;
}
