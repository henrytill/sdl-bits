#include <assert.h>
#include <math.h>
#include <stdint.h>

#ifdef _MSC_VER
#include <malloc.h>
#else
#include <stdlib.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#define print_error(error)                                                                         \
	do {                                                                                       \
		fprintf(stderr, "ERROR: %s:%d: %d", __FILE__, __LINE__, error);                    \
	} while (0)

#define EXPECTED_CHAR_BIT 8

enum { WIDTH = 10, HEIGHT = 20 };

static char *const FONT_FILE = "./assets/ucs-fonts/10x20.bdf";
static const char  CHAR_CODE = 'Q';

static unsigned char
get_bit(unsigned char x, unsigned int p);

static int
render_char(FT_GlyphSlot slot, unsigned char **target, size_t offset);

// p = 0 is MSB
static unsigned char
get_bit(unsigned char x, unsigned int p)
{
	assert(CHAR_BIT == EXPECTED_CHAR_BIT);
	if (p >= CHAR_BIT) {
		return 0;
	}
	return (x >> (CHAR_BIT + ~p)) & 1;
}

static int
render_char(FT_GlyphSlot slot, unsigned char **target, size_t offset)
{
	unsigned char *buffer = NULL;
	unsigned int   rows;
	unsigned int   width;
	unsigned char  datum;
	unsigned int   pitch;
	size_t         x, y, p, i;
	unsigned int   j;

	(void)offset;

	buffer = slot->bitmap.buffer;
	rows   = slot->bitmap.rows;
	width  = slot->bitmap.width;
	pitch  = abs(slot->bitmap.pitch);

	for (y = 0, p = 0; y < rows; y++, p += pitch) {
		for (i = 0; i < pitch; i++) {
			for (j = 0; j < CHAR_BIT; j++) {
				datum = get_bit(*(buffer + p + i), j);

				x = j + (i * CHAR_BIT);
				if (x < width) {
					*(*(target + y) + x) = datum;
				}
			}
		}
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int             ret     = 1;
	FT_Library      library = NULL;
	FT_Face         face    = NULL;
	FT_GlyphSlot    slot    = NULL;
	unsigned char **image   = NULL;
	unsigned int    rows;
	unsigned int    width;
	unsigned char   datum;
	int             error;

	(void)argc;
	(void)argv;

	setbuf(stdout, NULL);

	assert(CHAR_BIT == EXPECTED_CHAR_BIT);

	size_t i;
	image = malloc(HEIGHT * sizeof(char *));
	for (i = 0; i < HEIGHT; i++) {
		*(image + i) = malloc(WIDTH * sizeof(char));
	}

	error = FT_Init_FreeType(&library);
	if (error != 0) {
		print_error(error);
		goto cleanup;
	}

	error = FT_New_Face(library, FONT_FILE, 0, &face);
	if (error == FT_Err_Unknown_File_Format) {
		print_error(error);
		goto cleanup;
	}
	if (error != 0) {
		print_error(error);
		goto cleanup;
	}

	slot = face->glyph;

	error = FT_Set_Pixel_Sizes(face, WIDTH, HEIGHT);
	if (error != 0) {
		print_error(error);
		goto cleanup;
	}

	error = FT_Load_Char(face, CHAR_CODE, FT_LOAD_NO_SCALE | FT_LOAD_MONOCHROME);
	if (error != 0) {
		print_error(error);
		goto cleanup;
	}

	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
	if (error != 0) {
		print_error(error);
		goto cleanup;
	}

	if (slot->format != FT_GLYPH_FORMAT_BITMAP) {
		fprintf(stderr, "format was not FL_GLYPH_FORMAT_BITMAP");
		goto cleanup;
	}

	if (slot->bitmap.pixel_mode != FT_PIXEL_MODE_MONO) {
		fprintf(stderr, "pixel_mode was not FL_PIXEL_MODE_MONO");
	}

	render_char(slot, image, 0);

	rows  = slot->bitmap.rows;
	width = slot->bitmap.width;

	size_t x, y;

	for (y = 0; y < rows; y++) {
		printf("%2zd|", y);
		for (x = 0; x < width; x++) {
			datum = image[y][x] ? '*' : ' ';
			putchar(datum);
		}
		printf("|\n");
	}

	ret = 0;

cleanup:
	FT_Done_Face(face);
	FT_Done_FreeType(library);
	return ret;
}
