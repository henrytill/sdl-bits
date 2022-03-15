#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <malloc.h>
#else
#include <stdlib.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#include "bmp.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#define EXPECTED_CHAR_BIT 8

#define checked_fclose(file)                                                                       \
	do {                                                                                       \
		if ((file) != NULL) {                                                              \
			fclose((file));                                                            \
		}                                                                                  \
	} while (0)

#define print_error(error)                                                                         \
	do {                                                                                       \
		fprintf(stderr, "ERROR: %s:%d: %d", __FILE__, __LINE__, error);                    \
	} while (0)

enum { WIDTH = 10, HEIGHT = 20 };

static char *const FONT_FILE  = "./assets/ucs-fonts/10x20.bdf";
static char *const CHAR_CODES = "ABCDEFGHIJK";

static char *const TEST_BMP = "test.bmp";
static char *const MODE     = "wb";

static unsigned char
get_bit(unsigned char source, size_t pos);

static int
alloc_image(unsigned char ***image, size_t height, size_t width);

static void
free_image(unsigned char ***image, size_t height);

static void
render_char(FT_GlyphSlot slot, unsigned char **target, size_t offset);

static void
draw_image(unsigned char **image, size_t image_width, size_t image_height);

static int
export_image(unsigned char **image, size_t image_width, size_t image_height);

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

static int
export_image(unsigned char **image, size_t image_width, size_t image_height)
{
	const bmp_pixel_ARGB32_t WHITE              = {0xFF, 0xFF, 0xFF, 0x00};
	const bmp_pixel_ARGB32_t BLACK              = {0x00, 0x00, 0x00, 0xFF};
	int                      ret                = 1;
	FILE                    *file_h             = NULL;
	bmp_color_space_triple_t color_space_triple = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	bmp_bitmap_v4_header_t   bitmap_v4_header;
	bmp_file_header_t        file_header;
	bmp_pixel_ARGB32_t      *target_buff = NULL;
	size_t                   target_buff_size;
	size_t                   image_size_bytes;
	size_t                   writes;

	target_buff_size = image_width * image_height;
	image_size_bytes = target_buff_size * sizeof(bmp_pixel_ARGB32_t);

	target_buff = calloc(target_buff_size, sizeof(bmp_pixel_ARGB32_t));
	if (target_buff == NULL) {
		goto cleanup;
	}

	for (size_t y = image_height, i = 0; y-- > 0;) {
		for (size_t x = 0; x < image_width; x++, i++) {
			*(target_buff + i) = *(*(image + y) + x) ? BLACK : WHITE;
		}
	}

	bitmap_v4_header.width_px           = (int32_t)image_width;
	bitmap_v4_header.height_px          = (int32_t)image_height;
	bitmap_v4_header.num_planes         = 1;
	bitmap_v4_header.bits_per_pixel     = 32;
	bitmap_v4_header.compression        = 0x0003;
	bitmap_v4_header.image_size_bytes   = (uint32_t)image_size_bytes;
	bitmap_v4_header.x_resolution_ppm   = 0;
	bitmap_v4_header.y_resolution_ppm   = 0;
	bitmap_v4_header.num_colors         = 0;
	bitmap_v4_header.important_colors   = 0;
	bitmap_v4_header.red_mask           = 0x00FF0000;
	bitmap_v4_header.green_mask         = 0x0000FF00;
	bitmap_v4_header.blue_mask          = 0x000000FF;
	bitmap_v4_header.alpha_mask         = 0xFF000000;
	bitmap_v4_header.color_space_type   = 0x57696E20;
	bitmap_v4_header.color_space_triple = color_space_triple;
	bitmap_v4_header.red_gamma          = 0;
	bitmap_v4_header.green_gamma        = 0;
	bitmap_v4_header.blue_gamma         = 0;

	file_header.type            = 0x4D42;
	file_header.size            = (uint32_t)(bmp_bitmap_v4_offset + image_size_bytes);
	file_header.reserved1       = 0;
	file_header.reserved2       = 0;
	file_header.offset          = (uint32_t)bmp_bitmap_v4_offset;
	file_header.dib_header_size = BITMAPV4HEADER;

	file_h = fopen(TEST_BMP, MODE);
	if (file_h == NULL) {
		goto cleanup;
	}

	writes = fwrite(&file_header, sizeof(bmp_file_header_t), 1, file_h);
	if (writes != 1) {
		goto cleanup;
	}

	writes = fwrite(&bitmap_v4_header, sizeof(bmp_bitmap_v4_header_t), 1, file_h);
	if (writes != 1) {
		goto cleanup;
	}

	writes = fwrite(target_buff, image_size_bytes, 1, file_h);
	if (writes != 1) {
		goto cleanup;
	}

	ret = 0;

cleanup:
	checked_fclose(file_h);
	free(target_buff);
	return ret;
}

int
main(int argc, char *argv[])
{
	int             error;
	FT_Library      library = NULL;
	FT_Face         face    = NULL;
	FT_GlyphSlot    slot    = NULL;
	unsigned char **image   = NULL;
	size_t          char_codes_len;
	size_t          image_width;
	size_t          image_height;

	(void)argc;
	(void)argv;

	setbuf(stdout, NULL);

	assert(CHAR_BIT == EXPECTED_CHAR_BIT);

	char_codes_len = strlen(CHAR_CODES);

	image_width  = WIDTH * char_codes_len;
	image_height = HEIGHT;
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

	error = FT_Set_Pixel_Sizes(face, WIDTH, HEIGHT);
	if (error != 0) {
		print_error(error);
		goto cleanup;
	}

	for (size_t i = 0; i < char_codes_len; i++) {
		error = FT_Load_Char(face, CHAR_CODES[i], FT_LOAD_NO_SCALE | FT_LOAD_MONOCHROME);
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

	draw_image(image, image_width, image_height);

	export_image(image, image_width, image_height);

cleanup:
	FT_Done_Face(face);
	FT_Done_FreeType(library);
	free_image(&image, image_height);
	return error;
}
