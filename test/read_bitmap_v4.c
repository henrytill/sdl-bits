#include <stddef.h>
#include <stdlib.h>

#include "test.h"
#include "bmp.h"

enum {
	SUCCESS = 0
};

static const char *const BMP_FILE = "./assets/test.bmp";

int
main(int argc, char *argv[])
{
	char *image = NULL;
	struct bmp_FileHeader file_header;
	struct bmp_BitmapV4Header bitmap_v4_header;
	int rc;

	(void)argc;
	(void)argv;

	rc = bmp_read_bitmap_v4(BMP_FILE, &file_header, &bitmap_v4_header,
		&image);
	if (rc != SUCCESS) {
		goto out;
	}

	struct bmp_PixelARGB32 *pixel = (struct bmp_PixelARGB32 *)image;

	if (pixel->blue != 255) {
		print_failure(__FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out;
	}
	if (pixel->green != 0) {
		print_failure(__FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out;
	}
	if (pixel->red != 0) {
		print_failure(__FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out;
	}
	if (pixel->alpha != 127) {
		print_failure(__FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out;
	}
out:
	free(image);
	return rc;
}
