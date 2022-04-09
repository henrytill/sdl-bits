#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <SDL.h>

#include "util.h"

enum {
    SUCCESS = 0,
    FAILURE = 1,
};

enum { UNHANDLED = SDL_LOG_CATEGORY_CUSTOM };

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
    SDL_Renderer *renderer;
};

static const char *const window_title = "Hello, world!";

static struct Config default_config = {
    .window_width_pixels = 640,
    .window_height_pixels = 480,
    .target_frame_rate = 60,
};

static const char *const TEST_BMP = "../../assets/test.bmp";

static inline void log_sdl_error(int category, char *file, int line) {
    const char *sdl_err = SDL_GetError();
    if (strlen(sdl_err) != 0) {
        SDL_LogError((category), "%s:%d: %s", file, line, sdl_err);
    } else {
        SDL_LogError((category), "%s:%d", file, line);
    }
}

static inline float calculate_frame_time_millis(unsigned int frames_per_second) {
    const float second_millis = 1000;
    return second_millis / (float)frames_per_second;
}

static int load_bmp(const char *file, SDL_Surface **out) {
    *out = SDL_LoadBMP(file);
    if (*out == NULL) {
        log_sdl_error(UNHANDLED, __FILE__, __LINE__);
        return FAILURE;
    }
    return SUCCESS;
}

static int create_main_window(struct Config *config, const char *title, struct MainWindow *out) {
    out->window = SDL_CreateWindow(title,
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   (int)config->window_width_pixels,
                                   (int)config->window_height_pixels,
                                   SDL_WINDOW_SHOWN);
    if (out->window == NULL) {
        log_sdl_error(UNHANDLED, __FILE__, __LINE__);
        return FAILURE;
    }
    out->renderer = SDL_CreateRenderer(out->window, -1, SDL_RENDERER_ACCELERATED);
    if (out->renderer == NULL) {
        log_sdl_error(UNHANDLED, __FILE__, __LINE__);
        return FAILURE;
    }
    SDL_SetRenderDrawColor(out->renderer, 0x00, 0x00, 0x00, 0xFF);
    return SUCCESS;
}

static inline void destroy_main_window(struct MainWindow *main_window) {
    if (main_window == NULL) {
        return;
    }
    if (main_window->renderer != NULL) {
        SDL_DestroyRenderer(main_window->renderer);
    }
    if (main_window->window != NULL) {
        SDL_DestroyWindow(main_window->window);
    }
}

static inline void destroy_texture(SDL_Texture *texture) {
    if (texture != NULL) {
        SDL_DestroyTexture(texture);
    }
}

static inline void free_surface(SDL_Surface *surface) {
    if (surface != NULL) {
        SDL_FreeSurface(surface);
    }
}

int main(int argc, char *argv[]) {
    int error;
    enum LoopStatus event_loop_status = RUN;
    struct MainWindow main_window = {.window = NULL, .renderer = NULL};
    SDL_Surface *test_bmp_surface = NULL;
    SDL_Texture *test_bmp_texture = NULL;
    SDL_Rect window_rect = {.x = 0,
                            .y = 0,
                            .w = (int)default_config.window_width_pixels,
                            .h = (int)default_config.window_height_pixels};
    SDL_Event event;
    uint32_t loop_start;
    uint32_t loop_duration;
    uint32_t frame_delay;

    (void)argc;
    (void)argv;

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);

    const float frame_time_millis = calculate_frame_time_millis(default_config.target_frame_rate);

    error = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    if (error != SUCCESS) {
        log_sdl_error(UNHANDLED, __FILE__, __LINE__);
        goto out;
    }

    error = create_main_window(&default_config, window_title, &main_window);
    if (error != SUCCESS) {
        goto out;
    }

    error = load_bmp(TEST_BMP, &test_bmp_surface);
    if (error != SUCCESS) {
        goto out;
    }

    test_bmp_texture = SDL_CreateTextureFromSurface(main_window.renderer, test_bmp_surface);
    if (test_bmp_texture == NULL) {
        log_sdl_error(UNHANDLED, __FILE__, __LINE__);
        error = FAILURE;
        goto out;
    }
    free_surface(test_bmp_surface);
    test_bmp_surface = NULL;

    while (event_loop_status == RUN) {
        loop_start = SDL_GetTicks();

        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
            case SDL_QUIT:
                event_loop_status = STOP;
                break;
            default:
                break;
            }
        }

        error = SDL_RenderClear(main_window.renderer);
        if (error != SUCCESS) {
            log_sdl_error(UNHANDLED, __FILE__, __LINE__);
            goto out;
        }

        error = SDL_RenderCopy(main_window.renderer, test_bmp_texture, NULL, &window_rect);
        if (error != SUCCESS) {
            log_sdl_error(UNHANDLED, __FILE__, __LINE__);
            goto out;
        }

        SDL_RenderPresent(main_window.renderer);

        loop_duration = SDL_GetTicks() - loop_start;
        frame_delay = util_uint32_sat_sub((uint32_t)frame_time_millis, loop_duration);
        if (frame_delay > 0) {
            SDL_Delay(frame_delay);
        }
    }
out:
    destroy_texture(test_bmp_texture);
    free_surface(test_bmp_surface);
    destroy_main_window(&main_window);
    SDL_Quit();
    return error;
}
