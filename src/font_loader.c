#include <ft2build.h>
#include FT_FREETYPE_H

static char *const font_file = "./assets/ucs-fonts/6x13.bdf";

int
main(int argc, char *argv[])
{
	int        ret     = 1;
	FT_Library library = NULL;
	FT_Face    face    = NULL;
	int        error;

	(void)argc;
	(void)argv;

	error = FT_Init_FreeType(&library);
	if (error != 0) {
		fprintf(stderr, "%s:%d: %d", __FILE__, __LINE__, error);
		goto cleanup;
	}

	error = FT_New_Face(library, font_file, 0, &face);
	if (error == FT_Err_Unknown_File_Format) {
		fprintf(stderr, "%s:%d: %d", __FILE__, __LINE__, error);
		goto cleanup;
	}
	if (error != 0) {
		fprintf(stderr, "%s:%d: %d", __FILE__, __LINE__, error);
		goto cleanup;
	}

	ret = 0;

cleanup:
	return ret;
}
