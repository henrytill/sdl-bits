#include <stddef.h>
#include <stdlib.h>

#include "bmp.h"
#include "test.h"

static const char *const BMP_FILE = "./assets/test.bmp";

int main(int argc, char *argv[]) {
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

    test(pixel->blue == 255);
    test(pixel->green == 0);
    test(pixel->red == 0);
    test(pixel->alpha == 127);

out:
    free(image);
    return error;
}
