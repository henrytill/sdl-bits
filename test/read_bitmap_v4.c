#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "test.h"

static char *const BMP_FILE = "./assets/test.bmp";

int
main(int argc, char *argv[])
{
	int                    ret   = 1;
	char                  *image = NULL;
	bmp_file_header_t      file_header;
	bmp_bitmap_v4_header_t bitmap_v4_header;
	int                    error;

	(void)argc;
	(void)argv;

	error = bmp_read_bitmap_v4(BMP_FILE, &file_header, &bitmap_v4_header, &image);
	if (error != 0) {
		goto cleanup;
	}

	bmp_pixel_ARGB32_t *pixel = (bmp_pixel_ARGB32_t *)image;

	TEST(pixel->blue == 255);
	TEST(pixel->green == 0);
	TEST(pixel->red == 0);
	TEST(pixel->alpha == 127);

	ret = 0;

cleanup:
	free(image);
	return ret;
}
