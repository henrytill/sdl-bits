#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#include "bmp.h"
#include "test.h"

static char *const BMP_FILE = "./assets/sample_24bit.bmp";
static char *const MODE     = "r";

int
main(int argc, char *argv[])
{
	int                       ret                  = 1;
	FILE                     *file_h               = NULL;
	bmp_file_header_t        *file_header_h        = NULL;
	bmp_bitmap_info_header_t *bitmap_info_header_h = NULL;
	uint32_t                  dib_header_size;
	uint32_t                  image_size_bytes;
	char                     *image;
	size_t                    reads;
	int                       error;
	fpos_t                    pos;

	(void)argc;
	(void)argv;

	file_h = fopen(BMP_FILE, MODE);
	if (file_h == NULL) {
		goto cleanup;
	}

	file_header_h = malloc(sizeof(bmp_file_header_t));
	if (file_header_h == NULL) {
		goto cleanup;
	}

	reads = fread(file_header_h, sizeof(bmp_file_header_t), 1, file_h);
	if (reads != 1) {
		goto cleanup;
	}

	error = fgetpos(file_h, &pos);
	if (error != 0) {
		goto cleanup;
	}

	reads = fread(&dib_header_size, sizeof(uint32_t), 1, file_h);
	if (reads != 1) {
		goto cleanup;
	}

	error = fsetpos(file_h, &pos);
	if (error != 0) {
		goto cleanup;
	}

	if (dib_header_size != (size_t)BITMAPINFOHEADER) {
		fprintf(stderr, "Unexpected DIB Header Size: %d", dib_header_size);
		goto cleanup;
	}

	bitmap_info_header_h = malloc(sizeof(bmp_bitmap_info_header_t));
	if (bitmap_info_header_h == NULL) {
		goto cleanup;
	}

	reads = fread(bitmap_info_header_h, sizeof(bmp_bitmap_info_header_t), 1, file_h);
	if (reads != 1) {
		goto cleanup;
	}

	image_size_bytes = bitmap_info_header_h->image_size_bytes;

	image = calloc(image_size_bytes, sizeof(char));

	reads = fread(image, image_size_bytes * sizeof(char), 1, file_h);
	if (reads != 1) {
		goto cleanup;
	}

	bmp_pixel_RGB24_t *pixel = (bmp_pixel_RGB24_t *)image;

	TEST(pixel->blue == 0);
	TEST(pixel->green == 0);
	TEST(pixel->red == 255);

	ret = 0;

cleanup:
	free(file_header_h);
	fclose(file_h);

	return ret;
}
