#include <stdio.h>
#include <SDL.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	const int      SCREEN_WIDTH   = 640;
	const int      SCREEN_HEIGHT  = 480;
	const uint32_t DELAY          = 2000;
	int            ret            = 1;
	SDL_Window    *window         = NULL;
	SDL_Surface   *window_surface = NULL;
	uint32_t       fill_color;

	(void)argc;
	(void)argv;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		util_print_sdl_error();
		goto cleanup;
	}

	window = SDL_CreateWindow("Hello, world!",
	                          SDL_WINDOWPOS_UNDEFINED,
	                          SDL_WINDOWPOS_UNDEFINED,
	                          SCREEN_WIDTH,
	                          SCREEN_HEIGHT,
	                          SDL_WINDOW_SHOWN);
	if (window == NULL) {
		util_print_sdl_error();
		goto cleanup;
	}

	window_surface = SDL_GetWindowSurface(window);
	if (window_surface == NULL) {
		util_print_sdl_error();
		goto cleanup;
	}

	fill_color = SDL_MapRGB(window_surface->format, 0xFF, 0xFF, 0xFF);

	if (SDL_FillRect(window_surface, NULL, fill_color) != 0) {
		util_print_sdl_error();
		goto cleanup;
	}

	if (SDL_UpdateWindowSurface(window) != 0) {
		util_print_sdl_error();
		goto cleanup;
	};

	SDL_Delay(DELAY);

	ret = 0;

cleanup:
	SDL_DestroyWindow(window);
	SDL_Quit();
	return ret;
}
