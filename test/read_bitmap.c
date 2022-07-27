#include <stddef.h>
#include <stdlib.h>

#include "bmp.h"
#include "test.h"

enum {
	SUCCESS = 0
};

static const char *const BMP_FILE = "./assets/sample_24bit.bmp";

int
main(int argc, char *argv[])
{
	char *image = NULL;
	struct bmp_Filehdr filehdr;
	struct bmp_Infohdr infohdr;
	int rc;

	(void)argc;
	(void)argv;

	rc = bmp_read(BMP_FILE, &filehdr, &infohdr, &image);
	if (rc != SUCCESS) {
		goto out;
	}

	struct bmp_Pixel24 *pixel = (struct bmp_Pixel24 *)image;

	if (pixel->b != 0) {
		print_failure(__FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out;
	}
	if (pixel->g != 0) {
		print_failure(__FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out;
	}
	if (pixel->r != 255) {
		print_failure(__FILE__, __LINE__);
		rc = EXIT_FAILURE;
		goto out;
	}
out:
	free(image);
	return rc;
}
