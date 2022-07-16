#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_ttf.h>

enum {
	SUCCESS = 0,
	FAILURE = 1,
};

enum {
	CODESZ = 94, // ('~' - '!') + 1
};

static const char *const FONTFILE =
	"./EBGaramond12/"
	"EBGaramond12-e608414f52e532b68e2182f96b4ce9db35335593/"
	"fonts/ttf/"
	"EBGaramond-Regular.ttf";

static const char *const BMPFILE = "./EBGaramond-Regular.bmp";

static const SDL_Color fgcolor = {0x00, 0x00, 0x00, 0xFF};

int
main(int argc, char *argv[])
{
	int rc = FAILURE;

	(void)argc;
	(void)argv;

	char *code = calloc(CODESZ + 1, sizeof(char));
	if (code == NULL) {
		return rc;
	}

	for (int i = 0; i < CODESZ; ++i) {
		code[i] = (char)(i + '!');
	}

	code[CODESZ] = '\0';

	printf("%s\n", code);

	rc = TTF_Init();
	if (rc != SUCCESS) {
		goto out;
	}

	TTF_Font *font = TTF_OpenFont(FONTFILE, 64);
	if (font == NULL) {
		rc = FAILURE;
		goto out;
	}

	TTF_SetFontKerning(font, 1);

	SDL_Surface *surface = TTF_RenderText_Blended(font, code, fgcolor);
	if (surface == NULL) {
		rc = FAILURE;
		goto out;
	}

	rc = SDL_SaveBMP(surface, BMPFILE);
out:
	TTF_Quit();
	free(code);
	return rc;
}
