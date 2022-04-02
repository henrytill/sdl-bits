#include <stddef.h>
#include <stdint.h>

#include <SDL.h>

#include "util.h"

enum LoopStatus {
    STOP = 0,
    RUN = 1
};

struct Config {
    unsigned int window_width_pixels;
    unsigned int window_height_pixels;
    unsigned int target_frame_rate;
};

struct MainWindow {
    SDL_Window *window;
    SDL_Surface *surface;
};

static const char *const window_title = "Hello, world!";

static struct Config default_config = {
    .window_width_pixels = 640,
    .window_height_pixels = 480,
    .target_frame_rate = 60,
};

static float calculate_frame_time_millis(unsigned int frames_per_second) {
    const float second_millis = 1000;
    return second_millis / (float)frames_per_second;
}

static int create_main_window(struct Config *config, const char *title, struct MainWindow *out) {
    out->window = SDL_CreateWindow(title,
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   (int)config->window_width_pixels,
                                   (int)config->window_height_pixels,
                                   SDL_WINDOW_SHOWN);
    if (out->window == NULL) {
        util_log_sdl_error(SDL_LOG_CATEGORY_ERROR);
        return 1;
    }
    out->surface = SDL_GetWindowSurface(out->window);
    if (out->surface == NULL) {
        util_log_sdl_error(SDL_LOG_CATEGORY_ERROR);
        return 1;
    }
    return 0;
}

static void destroy_main_window(struct MainWindow *main_window) {
    if (main_window != NULL && main_window->window != NULL) {
        SDL_DestroyWindow(main_window->window);
    }
}

static int fill_surface(SDL_Surface *surface, uint8_t red, uint8_t green, uint8_t blue) {
    uint32_t fill_color = SDL_MapRGB(surface->format, red, green, blue);

    int error = SDL_FillRect(surface, NULL, fill_color);
    if (error != 0) {
        util_log_sdl_error(SDL_LOG_CATEGORY_ERROR);
    }
    return error;
}

int main(int argc, char *argv[]) {
    int error;
    enum LoopStatus event_loop_status = RUN;
    struct MainWindow main_window = {.window = NULL, .surface = NULL};
    SDL_Event event;

    (void)argc;
    (void)argv;

    const float frame_time_millis = calculate_frame_time_millis(default_config.target_frame_rate);

    error = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    if (error != 0) {
        util_log_sdl_error(SDL_LOG_CATEGORY_SYSTEM);
        goto out;
    }

    error = create_main_window(&default_config, window_title, &main_window);
    if (error != 0) {
        goto out;
    }

    error = fill_surface(main_window.surface, 0x00, 0x00, 0x00);
    if (error != 0) {
        goto out;
    }

    while (event_loop_status == RUN) {
        uint32_t loop_start = SDL_GetTicks();

        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
            case SDL_QUIT:
                event_loop_status = STOP;
                break;
            default:
                break;
            }
        }

        error = SDL_UpdateWindowSurface(main_window.window);
        if (error != 0) {
            util_log_sdl_error(SDL_LOG_CATEGORY_ERROR);
            goto out;
        }

        uint32_t loop_duration = SDL_GetTicks() - loop_start;
        uint32_t frame_delay = util_uint32_sat_sub((uint32_t)frame_time_millis, loop_duration);
        if (frame_delay > 0) {
            SDL_Delay(frame_delay);
        }
    }

out:
    destroy_main_window(&main_window);
    SDL_Quit();
    return error;
}
