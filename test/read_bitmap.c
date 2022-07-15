#include <stddef.h>
#include <stdlib.h>

#include "test.h"
#include "bmp.h"

enum {
	SUCCESS = 0
};

static const char *const BMP_FILE = "./assets/sample_24bit.bmp";

int
main(int argc, char *argv[])
{
	char *image = NULL;
	struct bmp_FileHeader file_header;
	struct bmp_BitmapInfoHeader bitmap_info_header;
	int error;

	(void)argc;
	(void)argv;

	error = bmp_read_bitmap(BMP_FILE, &file_header, &bitmap_info_header,
		&image);
	if (error != SUCCESS) {
		goto out;
	}

	struct bmp_PixelRGB24 *pixel = (struct bmp_PixelRGB24 *)image;

	if (pixel->blue != 0) {
		print_failure(__FILE__, __LINE__);
		error = EXIT_FAILURE;
		goto out;
	}
	if (pixel->green != 0) {
		print_failure(__FILE__, __LINE__);
		error = EXIT_FAILURE;
		goto out;
	}
	if (pixel->red != 255) {
		print_failure(__FILE__, __LINE__);
		error = EXIT_FAILURE;
		goto out;
	}
out:
	free(image);
	return error;
}
