#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "bmp.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#define EXPECTED_CHAR_BIT 8

#define print_error(error)                                                                         \
	do {                                                                                       \
		fprintf(stderr, "ERROR: %s:%d: %d", __FILE__, __LINE__, error);                    \
	} while (0)

enum {
	FONT_WIDTH      = 10,
	FONT_HEIGHT     = 20,
	CHAR_CODES_SIZE = 94, // ('~' - '!') + 1
};

static char *const              FONT_FILE = "./assets/ucs-fonts/10x20.bdf";
static char *const              TEST_BMP  = "test.bmp";
static const bmp_pixel_ARGB32_t WHITE     = {0xFF, 0xFF, 0xFF, 0x00};
static const bmp_pixel_ARGB32_t BLACK     = {0x00, 0x00, 0x00, 0xFF};

static unsigned char
get_bit(unsigned char source, size_t pos);

static int
alloc_image(unsigned char ***image, size_t height, size_t width);

static void
free_image(unsigned char ***image, size_t height);

static void
render_char(FT_GlyphSlot slot, unsigned char **target, size_t offset);

#ifdef DRAW_IMAGE
static void
draw_image(unsigned char **image, size_t image_width, size_t image_height);
#endif

// pos = 0 is MSB
static unsigned char
get_bit(unsigned char source, size_t pos)
{
	assert(CHAR_BIT == EXPECTED_CHAR_BIT);
	if (pos >= CHAR_BIT) {
		return 0;
	}
	// Also: source & (1 << (CHAR_BIT + ~pos));
	return (source >> (CHAR_BIT + ~pos)) & 1;
}

static int
alloc_image(unsigned char ***image, size_t height, size_t width)
{
	*image = calloc(height, sizeof(unsigned char *));
	if (*image == NULL) {
		return 1;
	}

	for (size_t i = 0; i < height; i++) {
		*(*image + i) = calloc(width, sizeof(unsigned char));
		if (*(*image + i) == NULL) {
			return 1;
		}
	}

	return 0;
}

void
free_image(unsigned char ***image, size_t height)
{
	if (image == NULL) {
		return;
	}

	for (size_t i = 0; i < height; i++) {
		free(*(*image + i));
	}
	free(*image);
	*image = NULL;
}

static void
render_char(FT_GlyphSlot slot, unsigned char **target, size_t offset)
{
	unsigned char *buffer = NULL;
	unsigned int   rows;
	unsigned int   width;
	unsigned char  datum;
	unsigned int   pitch;

	buffer = slot->bitmap.buffer;
	rows   = slot->bitmap.rows;
	width  = slot->bitmap.width;
	pitch  = abs(slot->bitmap.pitch);

	for (size_t y = 0, p = 0; y < rows; y++, p += pitch) {
		for (size_t i = 0; i < pitch; i++) {
			for (size_t j = 0, x; j < CHAR_BIT; j++) {
				datum = get_bit(*(buffer + p + i), j);

				x = j + (i * CHAR_BIT);
				if (x < width) {
					*(*(target + y) + x + (offset * width)) = datum;
				}
			}
		}
	}
}

#ifdef DRAW_IMAGE
static void
draw_image(unsigned char **image, size_t image_width, size_t image_height)
{
	for (size_t y = 0; y < image_height; y++) {
		printf("%2zd|", y);
		for (size_t x = 0; x < image_width; x++) {
			putchar(image[y][x] ? '*' : ' ');
		}
		printf("|\n");
	}
}
#endif

int
main(int argc, char *argv[])
{
	int                 error;
	FT_Library          library     = NULL;
	FT_Face             face        = NULL;
	FT_GlyphSlot        slot        = NULL;
	unsigned char     **image       = NULL;
	bmp_pixel_ARGB32_t *target_buff = NULL;
	char                char_codes[CHAR_CODES_SIZE];
	size_t              image_width;
	size_t              image_height;

	(void)argc;
	(void)argv;

	setbuf(stdout, NULL);

	// https://stackoverflow.com/a/881968
	assert(CHAR_BIT == EXPECTED_CHAR_BIT);

	for (size_t i = 0; i < CHAR_CODES_SIZE; i++) {
		char_codes[i] = (char)(i + '!');
	}

	image_width  = FONT_WIDTH * CHAR_CODES_SIZE;
	image_height = FONT_HEIGHT;
	error        = alloc_image(&image, image_height, image_width);
	if (error != 0) {
		fprintf(stderr, "could not allocate image");
		goto cleanup;
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

	error = FT_Set_Pixel_Sizes(face, FONT_WIDTH, FONT_HEIGHT);
	if (error != 0) {
		print_error(error);
		goto cleanup;
	}

	for (size_t i = 0; i < CHAR_CODES_SIZE; i++) {
		error = FT_Load_Char(face, char_codes[i], FT_LOAD_NO_SCALE | FT_LOAD_MONOCHROME);
		if (error != 0) {
			print_error(error);
			goto cleanup;
		}

		slot = face->glyph;

		error = FT_Render_Glyph(slot, FT_RENDER_MODE_MONO);
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
			goto cleanup;
		}

		render_char(slot, image, i);
	}

#ifdef DRAW_IMAGE
	draw_image(image, image_width, image_height);
#endif

	target_buff = calloc(image_width * image_height, sizeof(bmp_pixel_ARGB32_t));
	if (target_buff == NULL) {
		goto cleanup;
	}

	for (size_t y = image_height, i = 0; y-- > 0;) {
		for (size_t x = 0; x < image_width; x++, i++) {
			*(target_buff + i) = *(*(image + y) + x) ? BLACK : WHITE;
		}
	}

	bmp_write_bitmap_v4(target_buff, image_width, image_height, TEST_BMP);

cleanup:
	free(target_buff);
	FT_Done_Face(face);
	FT_Done_FreeType(library);
	free_image(&image, image_height);
	return error;
}
