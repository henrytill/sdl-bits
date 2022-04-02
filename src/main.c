#include <stddef.h>
#include <stdint.h>

#include <SDL.h>

#include "util.h"

/* Some types */

enum LoopStatus {
    STOP = 0,
    RUN  = 1
};

struct Config {
    unsigned int window_width_pixels;
    unsigned int window_height_pixels;
    unsigned int target_frame_rate;
};

struct MainWindow {
    SDL_Window  *window;
    SDL_Surface *surface;
};

/* Static Function Prototypes */

// Calculates frame time (in milliseconds) from the provided frames per second.
static float calculate_frame_time_millis(unsigned int frames_per_second);

// Calls SDL_CreateWindow and SDL_GetWindowSurface on the created window.  Stores pointers to the
// window and surface in the output struct.
static int create_main_window(struct Config *config, const char *title, struct MainWindow *out);

// Calls SDL_DestroyWindow on the provided main window's SDL_Window.
static void destroy_main_window(struct MainWindow *main_window);

// Fills a given surface with the provided color.
static int fill_surface(SDL_Surface *surface, uint8_t red, uint8_t green, uint8_t blue);

/* Static Function Definitions */

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

/* Static Data */

static const char *const window_title = "Hello, world!";

static struct Config config = {
    .window_width_pixels  = 640,
    .window_height_pixels = 480,
    .target_frame_rate    = 60,
};

/* main */

int main(int argc, char *argv[]) {
    enum LoopStatus   event_loop_status = RUN;
    struct MainWindow main_window       = {.window = NULL, .surface = NULL};
    SDL_Event         event;
    uint32_t          loop_start;
    uint32_t          loop_end;
    uint32_t          frame_delay;
    int               error;

    (void)argc;
    (void)argv;

    const float target_frame_time_millis = calculate_frame_time_millis(config.target_frame_rate);

    error = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    if (error != 0) {
        util_log_sdl_error(SDL_LOG_CATEGORY_SYSTEM);
        goto out;
    }

    error = create_main_window(&config, window_title, &main_window);
    if (error != 0) {
        goto out;
    }

    error = fill_surface(main_window.surface, 0x00, 0x00, 0x00);
    if (error != 0) {
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
        error = SDL_UpdateWindowSurface(main_window.window);
        if (error != 0) {
            util_log_sdl_error(SDL_LOG_CATEGORY_ERROR);
            goto out;
        }

        // Calculate frame delay
        loop_end    = SDL_GetTicks() - loop_start;
        frame_delay = util_uint32_sat_sub((uint32_t)target_frame_time_millis, loop_end);
        if (frame_delay > 0) {
            SDL_Delay(frame_delay);
        }
    }
    // End main event loop

out:
    destroy_main_window(&main_window);
    SDL_Quit();
    return error;
}
