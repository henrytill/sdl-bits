#include <stdio.h>

#include "SDL.h"

const int      SCREEN_WIDTH  = 640;
const int      SCREEN_HEIGHT = 480;
const uint32_t DELAY         = 2000;

int
main(int argc, char *argv[])
{
	int          ret     = 1;
	SDL_Window  *window  = NULL;
	SDL_Surface *surface = NULL;
	uint32_t     fill_color;

	(void)argc;
	(void)argv;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		goto cleanup;
	}

	window = SDL_CreateWindow("Hello, world!",
	                          SDL_WINDOWPOS_UNDEFINED,
	                          SDL_WINDOWPOS_UNDEFINED,
	                          SCREEN_WIDTH,
	                          SCREEN_HEIGHT,
	                          SDL_WINDOW_SHOWN);
	if (window == NULL) {
		fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
		goto cleanup;
	}

	surface = SDL_GetWindowSurface(window);
	if (surface == NULL) {
		fprintf(stderr, "SDL_GetWindowSurface: %s\n", SDL_GetError());
		goto cleanup;
	}

	fill_color = SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF);

	if (SDL_FillRect(surface, NULL, fill_color) != 0) {
		fprintf(stderr, "SDL_FillRect: %s\n", SDL_GetError());
		goto cleanup;
	}

	if (SDL_UpdateWindowSurface(window) != 0) {
		fprintf(
		    stderr, "SDL_UpdateWindowSurface: %s\n", SDL_GetError());
		goto cleanup;
	};

	SDL_Delay(DELAY);

	ret = 0;

cleanup:
	SDL_DestroyWindow(window);
	SDL_Quit();
	return ret;
}
