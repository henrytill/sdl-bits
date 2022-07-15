#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

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

enum WindowType {
	WINDOWED = 0,
	FULLSCREEN = 1,
	FULLSCREEN_BORDERLESS = 2,
	NUM_WINDOW_TYPES
};

struct Arguments {
	char *config_path;
};

struct Config {
	enum WindowType window_type;
	int window_width_pixels;
	int window_height_pixels;
	int target_frame_rate;
	char *asset_path;
};

struct MainWindow {
	SDL_Window *window;
	SDL_Renderer *renderer;
};

static const float MS_PER_SECOND = 1000.0f;

static const char *const ARG_CONFIG_PATH = "--config-path";

static const char *const WINDOW_TITLE = "Hello, world!";

static const char *const TEST_BMP_FILE = "test.bmp";

static uint64_t counter_freq_hz = 0;

static const uint32_t base_window_flags[] = {
	[WINDOWED] = SDL_WINDOW_SHOWN,
	[FULLSCREEN] = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN,
	[FULLSCREEN_BORDERLESS] =
		SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP,
};

static const char *const window_description[] = {
	[WINDOWED] = "Windowed",
	[FULLSCREEN] = "Fullscreen",
	[FULLSCREEN_BORDERLESS] = "Borderless Fullscreen",
};

static struct Arguments default_args = {.config_path = "config.lua"};

static struct Config default_config = {
	.window_type = WINDOWED,
	.window_width_pixels = 1280,
	.window_height_pixels = 720,
	.target_frame_rate = 60,
	.asset_path = "./assets",
};

static inline uint64_t
now(void)
{
	return SDL_GetPerformanceCounter();
}

static void
log_sdl_error(int category, char *file, int line)
{
	const char *sdl_err = SDL_GetError();
	if (strlen(sdl_err) != 0) {
		SDL_LogError(category, "%s:%d: %s", file, line, sdl_err);
	} else {
		SDL_LogError(category, "%s:%d", file, line);
	}
}

static void
parse_args(int argc, char *argv[], struct Arguments *args)
{
	for (int i = 0; i < argc;) {
		char *arg = argv[i++];
		if (strcmp(arg, ARG_CONFIG_PATH) == 0) {
			args->config_path = argv[i++];
		}
	}
}

static char *
init_asset_path(const struct Config *config, const char *sub_path)
{
	size_t len = (size_t)
		snprintf(NULL, 0, "%s/%s", config->asset_path, sub_path);
	len += 1; /* for null-termination */
	char *ret = calloc(len, sizeof(char));
	if (ret != NULL) {
		snprintf(ret, len, "%s/%s", config->asset_path, sub_path);
	}
	return ret;
}

static int
config_load(const char *filename, struct Config *config)
{
	int error = FAILURE;

	lua_State *state = luaL_newstate();
	if (state == NULL) {
		SDL_LogError(UNHANDLED,
			"%s: failed to initialize Lua",
			__func__);
		return error;
	}

	luaL_openlibs(state);

	error = luaL_loadfile(state, filename) || lua_pcall(state, 0, 0, 0);
	if (error != LUA_OK) {
		SDL_LogError(UNHANDLED,
			"%s: failed to load file: %s, %s",
			__func__,
			filename,
			lua_tostring(state, -1));
		lua_pop(state, 1);
		error = FAILURE;
		goto out;
	}

	lua_getglobal(state, "width");
	lua_getglobal(state, "height");
	if (!lua_isnumber(state, -2)) {
		SDL_LogError(UNHANDLED, "%s: width is not a number", __func__);
		error = FAILURE;
		goto out;
	}
	if (!lua_isnumber(state, -1)) {
		SDL_LogError(UNHANDLED, "%s: height is not a number", __func__);
		error = FAILURE;
		goto out;
	}
	config->window_width_pixels = (int)lua_tonumber(state, -2);
	config->window_height_pixels = (int)lua_tonumber(state, -1);

	error = SUCCESS;
out:
	lua_close(state);
	return error;
}

static float
calculate_frame_time_ms(int frames_per_second)
{
	assert(frames_per_second > 0);
	return MS_PER_SECOND / (float)frames_per_second;
}

static float
calculate_delta_ms(uint64_t begin_ticks, uint64_t end_ticks)
{
	assert(counter_freq_hz > 0);
	const float delta_ticks = (float)(end_ticks - begin_ticks);
	return (delta_ticks * MS_PER_SECOND) / (float)counter_freq_hz;
}

static void
delay(float target_frame_time_ms, uint64_t begin_ticks)
{
	if (calculate_delta_ms(begin_ticks, now()) < target_frame_time_ms) {
		const float delay_ms = target_frame_time_ms
				     - calculate_delta_ms(begin_ticks, now())
				     - 1.0f;
		if ((uint32_t)delay_ms > 0) {
			SDL_Delay((uint32_t)delay_ms);
		}
		while (calculate_delta_ms(begin_ticks, now())
			< target_frame_time_ms) {
			/* wait */
		}
	}
}

static int
load_bmp(const char *file, SDL_Surface **out)
{
	*out = SDL_LoadBMP(file);
	if (*out == NULL) {
		log_sdl_error(UNHANDLED, __FILE__, __LINE__);
		return FAILURE;
	}
	return SUCCESS;
}

static int
init_main_window(struct Config *config,
	const char *title,
	struct MainWindow *out)
{
	uint32_t window_flags = base_window_flags[config->window_type];
	uint32_t renderer_flags = SDL_RENDERER_ACCELERATED;
	SDL_LogInfo(UNHANDLED,
		"Window type: %s\n",
		window_description[config->window_type]);
	out->window = SDL_CreateWindow(title,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		config->window_width_pixels,
		config->window_height_pixels,
		window_flags);
	if (out->window == NULL) {
		log_sdl_error(UNHANDLED, __FILE__, __LINE__);
		return FAILURE;
	}
	out->renderer = SDL_CreateRenderer(out->window, -1, renderer_flags);
	if (out->renderer == NULL) {
		log_sdl_error(UNHANDLED, __FILE__, __LINE__);
		return FAILURE;
	}
	SDL_SetRenderDrawColor(out->renderer, 0x00, 0x00, 0x00, 0xFF);
	return SUCCESS;
}

static int
get_window_rect(struct MainWindow *main_window, SDL_Rect *out)
{
	int width;
	int height;

	if (main_window == NULL || main_window->renderer == NULL) {
		return FAILURE;
	}
	int error = SDL_GetRendererOutputSize(main_window->renderer,
		&width,
		&height);
	if (error != SUCCESS) {
		log_sdl_error(UNHANDLED, __FILE__, __LINE__);
		return error;
	}
	out->x = 0;
	out->y = 0;
	out->w = width;
	out->h = height;
	return SUCCESS;
}

static void
destroy_main_window(struct MainWindow *main_window)
{
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

static void
destroy_texture(SDL_Texture *texture)
{
	if (texture != NULL) {
		SDL_DestroyTexture(texture);
	}
}

static void
free_surface(SDL_Surface *surface)
{
	if (surface != NULL) {
		SDL_FreeSurface(surface);
	}
}

static void
handle_keydown(SDL_KeyboardEvent *key, enum LoopStatus *loop_status)
{
	switch (key->keysym.sym) {
	case SDLK_ESCAPE:
		*loop_status = STOP;
		break;
	default:
		break;
	}
}

static void
update(float delta_ms)
{
	(void)delta_ms;
}

int
main(int argc, char *argv[])
{
	int error = FAILURE;
	enum LoopStatus main_loop_status = RUN;
	struct MainWindow main_window = {.window = NULL, .renderer = NULL};
	SDL_Surface *test_bmp_surface = NULL;
	SDL_Texture *test_bmp_texture = NULL;
	SDL_Rect main_window_rect;
	SDL_Event event;
	uint64_t begin_ticks;
	uint64_t end_ticks;
	float delta_time_ms;

	(void)argc;
	(void)argv;

	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

	parse_args(argc, argv, &default_args);

	config_load(default_args.config_path, &default_config);

	char *test_bmp_path = init_asset_path(&default_config, TEST_BMP_FILE);
	if (test_bmp_path == NULL) {
		goto out;
	}

	const float target_frame_time_ms =
		calculate_frame_time_ms(default_config.target_frame_rate);

	error = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if (error != SUCCESS) {
		log_sdl_error(UNHANDLED, __FILE__, __LINE__);
		goto out;
	}

	counter_freq_hz = SDL_GetPerformanceFrequency();

	error = init_main_window(&default_config, WINDOW_TITLE, &main_window);
	if (error != SUCCESS) {
		goto out;
	}

	error = get_window_rect(&main_window, &main_window_rect);
	if (error != SUCCESS) {
		goto out;
	}

	error = load_bmp(test_bmp_path, &test_bmp_surface);
	if (error != SUCCESS) {
		goto out;
	}

	test_bmp_texture = SDL_CreateTextureFromSurface(main_window.renderer,
		test_bmp_surface);
	if (test_bmp_texture == NULL) {
		log_sdl_error(UNHANDLED, __FILE__, __LINE__);
		error = FAILURE;
		goto out;
	}
	free_surface(test_bmp_surface);
	test_bmp_surface = NULL;

	delta_time_ms = target_frame_time_ms;
	begin_ticks = now();
	while (main_loop_status == RUN) {
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
			case SDL_QUIT:
				main_loop_status = STOP;
				break;
			case SDL_KEYDOWN:
				handle_keydown(&event.key, &main_loop_status);
				break;
			default:
				break;
			}
		}

		update(delta_time_ms);

		/* render */
		{
			error = SDL_RenderClear(main_window.renderer);
			if (error != SUCCESS) {
				log_sdl_error(UNHANDLED, __FILE__, __LINE__);
				goto out;
			}

			error = SDL_RenderCopy(main_window.renderer,
				test_bmp_texture,
				NULL,
				&main_window_rect);
			if (error != SUCCESS) {
				log_sdl_error(UNHANDLED, __FILE__, __LINE__);
				goto out;
			}

			SDL_RenderPresent(main_window.renderer);
		}

		delay(target_frame_time_ms, begin_ticks);

		end_ticks = now();
		delta_time_ms = calculate_delta_ms(begin_ticks, end_ticks);
		begin_ticks = end_ticks;
	}
out:
	destroy_texture(test_bmp_texture);
	free_surface(test_bmp_surface);
	destroy_main_window(&main_window);
	SDL_Quit();
	free(test_bmp_path);
	return error;
}
