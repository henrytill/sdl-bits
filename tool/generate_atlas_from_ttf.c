#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_ttf.h>

enum {
	SUCCESS = 0,
	FAILURE = 1,
};

enum {
	CHAR_CODES_SIZE = 94, // ('~' - '!') + 1
};

static const char *const FONT_FILE =
	"./EBGaramond12/"
	"EBGaramond12-e608414f52e532b68e2182f96b4ce9db35335593/"
	"fonts/ttf/"
	"EBGaramond-Regular.ttf";

static const char *const BMP_FILE = "./EBGaramond-Regular.bmp";

static const SDL_Color color_foreground = {0x00, 0x00, 0x00, 0xFF};

int
main(int argc, char *argv[])
{
	int error = FAILURE;

	(void)argc;
	(void)argv;

	char *char_codes = calloc(CHAR_CODES_SIZE + 1, sizeof(char));
	if (char_codes == NULL) {
		return error;
	}

	for (int i = 0; i < CHAR_CODES_SIZE; ++i) {
		char_codes[i] = (char)(i + '!');
	}

	char_codes[CHAR_CODES_SIZE] = '\0';

	printf("%s\n", char_codes);

	error = TTF_Init();
	if (error != SUCCESS) {
		goto out;
	}

	TTF_Font *font = TTF_OpenFont(FONT_FILE, 64);
	if (font == NULL) {
		error = FAILURE;
		goto out;
	}

	TTF_SetFontKerning(font, 1);

	SDL_Surface *surface =
		TTF_RenderText_Blended(font, char_codes, color_foreground);
	if (surface == NULL) {
		error = FAILURE;
		goto out;
	}

	error = SDL_SaveBMP(surface, BMP_FILE);
out:
	TTF_Quit();
	free(char_codes);
	return error;
}
