#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#define ARG_ASSET_PATH "--asset-path"

#define now() SDL_GetPerformanceCounter()

enum {
    SUCCESS = 0,
    FAILURE = 1,
};

enum {
    UNHANDLED = SDL_LOG_CATEGORY_CUSTOM
};

enum LoopStatus {
    STOP = 0,
    RUN = 1
};

struct Config {
    int window_width_pixels;
    int window_height_pixels;
    int target_frame_rate;
    char *asset_path;
};

struct MainWindow {
    SDL_Window *window;
    SDL_Renderer *renderer;
};

static const float SECOND_MS = 1000.0f;

static const char *const WINDOW_TITLE = "Hello, world!";

static const char *const TEST_BMP_FILE = "test.bmp";

static uint64_t perf_freq = 0;

static struct Config default_config = {
    .window_width_pixels = 640,
    .window_height_pixels = 480,
    .target_frame_rate = 60,
    .asset_path = "./assets",
};

static inline void log_sdl_error(int category, char *file, int line) {
    const char *sdl_err = SDL_GetError();
    if (strlen(sdl_err) != 0) {
        SDL_LogError(category, "%s:%d: %s", file, line, sdl_err);
    } else {
        SDL_LogError(category, "%s:%d", file, line);
    }
}

static void parse_args(int argc, char *argv[], struct Config *config) {
    for (int i = 0; i < argc;) {
        char *arg = argv[i++];
        if (strcmp(arg, ARG_ASSET_PATH) == 0) {
            config->asset_path = argv[i];
        }
    }
}

static char *init_asset_path(const struct Config *config, const char *sub_path) {
    size_t len = (size_t)snprintf(NULL, 0, "%s/%s", config->asset_path, sub_path);
    len += 1; /* for null-termination */
    char *ret = calloc(len, sizeof(char));
    if (ret != NULL) {
        snprintf(ret, len, "%s/%s", config->asset_path, sub_path);
    }
    return ret;
}

static inline float calculate_frame_time_ms(int frames_per_second) {
    assert(frames_per_second > 0);
    return SECOND_MS / (float)frames_per_second;
}

static inline float get_duration_ms(uint64_t last, uint64_t current) {
    assert(perf_freq != 0);
    const uint64_t duration_ticks = current - last;
    return ((float)duration_ticks * SECOND_MS) / (float)perf_freq;
}

static inline void delay(float target_frame_time_ms, uint64_t begin_ticks) {
    if (get_duration_ms(begin_ticks, now()) < target_frame_time_ms) {
        const float delay_ms = target_frame_time_ms - get_duration_ms(begin_ticks, now());
        if ((uint32_t)delay_ms > 0) {
            SDL_Delay((uint32_t)delay_ms);
        }
        while (get_duration_ms(begin_ticks, now()) < target_frame_time_ms) {
            /* wait */
        }
    }
}

static int load_bmp(const char *file, SDL_Surface **out) {
    *out = SDL_LoadBMP(file);
    if (*out == NULL) {
        log_sdl_error(UNHANDLED, __FILE__, __LINE__);
        return FAILURE;
    }
    return SUCCESS;
}

static int init_main_window(struct Config *config, const char *title, struct MainWindow *out) {
    out->window = SDL_CreateWindow(title,
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   config->window_width_pixels,
                                   config->window_height_pixels,
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
    int error = FAILURE;
    enum LoopStatus event_loop_status = RUN;
    struct MainWindow main_window = {.window = NULL, .renderer = NULL};
    SDL_Surface *test_bmp_surface = NULL;
    SDL_Texture *test_bmp_texture = NULL;
    SDL_Rect window_rect = {.x = 0,
                            .y = 0,
                            .w = default_config.window_width_pixels,
                            .h = default_config.window_height_pixels};
    SDL_Event event;
    uint64_t begin_ticks;

    (void)argc;
    (void)argv;

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);

    parse_args(argc, argv, &default_config);

    char *test_bmp_path = init_asset_path(&default_config, TEST_BMP_FILE);
    if (test_bmp_path == NULL) {
        goto out;
    }

    const float target_frame_time_ms = calculate_frame_time_ms(default_config.target_frame_rate);

    error = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    if (error != SUCCESS) {
        log_sdl_error(UNHANDLED, __FILE__, __LINE__);
        goto out;
    }

    perf_freq = SDL_GetPerformanceFrequency();

    error = init_main_window(&default_config, WINDOW_TITLE, &main_window);
    if (error != SUCCESS) {
        goto out;
    }

    error = load_bmp(test_bmp_path, &test_bmp_surface);
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

    begin_ticks = now();
    while (event_loop_status == RUN) {
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

        delay(target_frame_time_ms, begin_ticks);

        begin_ticks = now();
    }
out:
    destroy_texture(test_bmp_texture);
    free_surface(test_bmp_surface);
    destroy_main_window(&main_window);
    SDL_Quit();
    free(test_bmp_path);
    return error;
}
