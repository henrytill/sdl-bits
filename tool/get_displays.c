#include <SDL.h>

enum {
	SUCCESS = 0,
	FAILURE = 1,
};

static int
enumdisp(void)
{
	SDL_DisplayMode mode;
	int rc = FAILURE;

	int ndisp = SDL_GetNumVideoDisplays();
	if (ndisp < 0) {
		const char *err = SDL_GetError();
		SDL_Log("Failed to get number of video displays: %s", err);
		return rc;
	}
	for (int i = 0; i < ndisp; ++i) {
		rc = SDL_GetCurrentDisplayMode(i, &mode);
		if (rc != SUCCESS) {
			const char *err = SDL_GetError();
			SDL_Log("Failed to get mode for display #%d: %s", i,
				err);
			return rc;
		}
		SDL_Log("Display #%d: mode is %dx%d @ %d hz.", i, mode.w,
			mode.h, mode.refresh_rate);
	}
	return SUCCESS;
}

int
main(int argc, char *argv[])
{
	int rc = FAILURE;

	(void)argc;
	(void)argv;

	SDL_Init(SDL_INIT_VIDEO);

	rc = enumdisp();

	SDL_Quit();
	return rc;
}
