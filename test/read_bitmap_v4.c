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
	struct bmp_Filehdr filehdr;
	struct bmp_V4hdr v4hdr;
	int rc;

	(void)argc;
	(void)argv;

	rc = bmp_v4read(BMP_FILE, &filehdr, &v4hdr, &image);
	if (rc != SUCCESS) {
		goto out;
	}

	struct bmp_Pixel32 *pixel = (struct bmp_Pixel32 *)image;

	if (pixel->b != 255) {
		print_failure(__FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out;
	}
	if (pixel->g != 0) {
		print_failure(__FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out;
	}
	if (pixel->r != 0) {
		print_failure(__FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out;
	}
	if (pixel->a != 127) {
		print_failure(__FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out;
	}
out:
	free(image);
	return rc;
}
