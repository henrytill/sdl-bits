#include <stdio.h>
#include <SDL.h>

#include "util.h"

/* Establishes loop conditionals */
typedef enum loop_status_e { STOP = 0, RUN = 1 } loop_status_t;

int
main(int argc, char *argv[])
{
	const int     SCREEN_WIDTH      = 640;
	const int     SCREEN_HEIGHT     = 480;
	int           ret               = 1;
	loop_status_t event_loop_status = RUN;
	SDL_Window   *window            = NULL;
	SDL_Surface  *window_surface    = NULL;
	SDL_Event     event;
	uint32_t      fill_color;

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

	/* Main event loop */
	while (event_loop_status == RUN) {
		/* Handle events on the SDL event queue */
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
			case SDL_QUIT:
				event_loop_status = STOP;
				break;
			default:
				break;
			}
		}

		if (SDL_UpdateWindowSurface(window) != 0) {
			util_print_sdl_error();
			goto cleanup;
		};
	}

	ret = 0;

cleanup:
	SDL_DestroyWindow(window);
	SDL_Quit();
	return ret;
}
