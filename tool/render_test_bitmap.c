#include "bmp.h"

enum {
    WIDTH_PIXELS  = 4,
    HEIGHT_PIXELS = 2,
};

static const bmp_pixel_ARGB32_t TARGET_BUFF[] = {
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

int main(int argc, char *argv[]) {
    int ret = 1;
    int error;

    (void)argc;
    (void)argv;

    error = bmp_write_bitmap_v4(TARGET_BUFF, WIDTH_PIXELS, HEIGHT_PIXELS, BMP_FILE);
    if (error != 0) {
        goto out;
    }

    ret = 0;

out:
    return ret;
}
