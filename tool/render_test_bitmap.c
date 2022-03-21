#include "bmp.h"

enum {
	WIDTH_PIXELS  = 4,
	HEIGHT_PIXELS = 2,
};

static const bmp_pixel_ARGB32_t target_buff[] = {
    {0xFF, 0x00, 0x00, 0x7F},
    {0x00, 0xFF, 0x00, 0x7F},
    {0x00, 0x00, 0xFF, 0x7F},
    {0xFF, 0xFF, 0xFF, 0x7F},
    {0xFF, 0x00, 0x00, 0xFF},
    {0x00, 0xFF, 0x00, 0xFF},
    {0x00, 0x00, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF},
};

static char *const BMP_FILE = "./test.bmp";

int
main(int argc, char *argv[])
{
	int ret = 1;
	int error;

	(void)argc;
	(void)argv;

	error = bmp_write_bitmap_v4(target_buff, WIDTH_PIXELS, HEIGHT_PIXELS, BMP_FILE);
	if (error != 0) {
		goto cleanup;
	}

	ret = 0;

cleanup:
	return ret;
}
