#include <SDL.h>

#include "macro.h"

static int enumdisp(void) {
  SDL_DisplayMode mode;
  int rc, numDisplays;

  numDisplays = SDL_GetNumVideoDisplays();
  if (numDisplays < 0) {
    const char *err = SDL_GetError();
    SDL_Log("Failed to get number of video displays: %s", err);
    return -1;
  }

  for (int i = 0; i < numDisplays; ++i) {
    rc = SDL_GetCurrentDisplayMode(i, &mode);
    if (rc != 0) {
      const char *err = SDL_GetError();
      SDL_Log("Failed to get mode for display #%d: %s", i, err);
      return -1;
    }
    SDL_Log("Display #%d: mode is %dx%d @ %d hz.", i, mode.w, mode.h, mode.refresh_rate);
  }

  return 0;
}

int main(_unused_ int argc, _unused_ char *argv[]) {
  int rc;

  rc = SDL_Init(SDL_INIT_VIDEO);
  if (rc != 0)
    return EXIT_FAILURE;

  AT_EXIT(SDL_Quit);

  rc = enumdisp();
  if (rc != 0)
    return EXIT_FAILURE;

  return 0;
}
