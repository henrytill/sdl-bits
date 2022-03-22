#include <stdio.h>
#include <SDL.h>

#include "util.h"

static const int      SCREEN_WIDTH_PIXELS      = 640;
static const int      SCREEN_HEIGHT_PIXELS     = 480;
static const uint32_t TARGET_FRAME_TIME_MILLIS = 16;

// Establishes loop conditionals
typedef enum loop_status_e { STOP = 0, RUN = 1 } loop_status_t;

int main(int argc, char *argv[]) {
    int           ret               = 1;
    loop_status_t event_loop_status = RUN;
    SDL_Window   *window            = NULL;
    SDL_Surface  *window_surface    = NULL;
    SDL_Event     event;
    uint32_t      fill_color;
    uint32_t      loop_start;
    uint32_t      loop_end;
    uint32_t      frame_delay;

    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        util_print_sdl_error();
        goto out;
    }

    window = SDL_CreateWindow("Hello, world!",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH_PIXELS,
                              SCREEN_HEIGHT_PIXELS,
                              SDL_WINDOW_SHOWN);
    if (window == NULL) {
        util_print_sdl_error();
        goto out;
    }

    window_surface = SDL_GetWindowSurface(window);
    if (window_surface == NULL) {
        util_print_sdl_error();
        goto out;
    }

    fill_color = SDL_MapRGB(window_surface->format, 0xFF, 0xFF, 0xFF);

    if (SDL_FillRect(window_surface, NULL, fill_color) != 0) {
        util_print_sdl_error();
        goto out;
    }

    // Begin main event loop
    while (event_loop_status == RUN) {
        loop_start = SDL_GetTicks();

        // Handle events on the SDL event queue
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
            case SDL_QUIT:
                event_loop_status = STOP;
                break;
            default:
                break;
            }
        }

        // Update
        if (SDL_UpdateWindowSurface(window) != 0) {
            util_print_sdl_error();
            goto out;
        }

        // Calculate frame delay
        loop_end    = SDL_GetTicks() - loop_start;
        frame_delay = util_uint32_sat_sub(TARGET_FRAME_TIME_MILLIS, loop_end);
        if (frame_delay > 0) {
            SDL_Delay(frame_delay);
        }
    }
    // End main event loop

    ret = 0;

out:
    SDL_DestroyWindow(window);
    SDL_Quit();
    return ret;
}
