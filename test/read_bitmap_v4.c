#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "test.h"

static char *const BMP_FILE = "./assets/test.bmp";

int main(int argc, char *argv[]) {
    int                ret   = 1;
    char              *image = NULL;
    bmp_FileHeader     file_header;
    bmp_BitmapV4Header bitmap_v4_header;
    int                error;

    (void)argc;
    (void)argv;

    error = bmp_read_bitmap_v4(BMP_FILE, &file_header, &bitmap_v4_header, &image);
    if (error != 0) {
        goto out;
    }

    bmp_PixelARGB32 *pixel = (bmp_PixelARGB32 *)image;

    TEST(pixel->blue == 255);
    TEST(pixel->green == 0);
    TEST(pixel->red == 0);
    TEST(pixel->alpha == 127);

    ret = 0;

out:
    free(image);
    return ret;
}
