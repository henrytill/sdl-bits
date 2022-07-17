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
		return EXIT_FAILURE;
	}

	for (int i = 0; i < CODESZ; ++i) {
		code[i] = (char)(i + '!');
	}

	code[CODESZ] = '\0';

	printf("%s\n", code);

	rc = TTF_Init();
	if (rc != SUCCESS) {
		rc = EXIT_FAILURE;
		goto out0;
	}

	TTF_Font *font = TTF_OpenFont(FONTFILE, 64);
	if (font == NULL) {
		rc = EXIT_FAILURE;
		goto out1;
	}

	TTF_SetFontKerning(font, 1);

	SDL_Surface *s = TTF_RenderText_Blended(font, code, fgcolor);
	if (s == NULL) {
		rc = EXIT_FAILURE;
		goto out2;
	}

	rc = SDL_SaveBMP(s, BMPFILE);
	if (rc != SUCCESS) {
		rc = EXIT_FAILURE;
		goto out3;
	}

	rc = EXIT_SUCCESS;
out3:
	SDL_FreeSurface(s);
out2:
	TTF_CloseFont(font);
out1:
	TTF_Quit();
out0:
	free(code);
	return rc;
}
