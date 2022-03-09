#include <ft2build.h>
#include FT_FREETYPE_H

int
main(int argc, char *argv[])
{
	FT_Library library = NULL;
	FT_Face    face    = NULL;
	int        ret     = 1;
	int        error;

	(void)argc;
	(void)argv;

	error = FT_Init_FreeType(&library);
	if (error != 0) {
		fprintf(stderr, "%s:%d: %d", __FILE__, __LINE__, error);
		ret = 1;
		goto cleanup;
	}

	error = FT_New_Face(library, "./assets/ucs-fonts/6x13.bdf", 0, &face);
	if (error == FT_Err_Unknown_File_Format) {
		fprintf(stderr, "%s:%d: Unknown file format", __FILE__, __LINE__);
		ret = 1;
		goto cleanup;
	}
	if (error != 0) {
		fprintf(stderr, "%s:%d: %d", __FILE__, __LINE__, error);
		ret = 1;
		goto cleanup;
	}

	ret = 0;

cleanup:
	return ret;
}