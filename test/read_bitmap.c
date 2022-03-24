#include <stddef.h>
#include <stdlib.h>

#include "bmp.h"
#include "test.h"

static char *const BMP_FILE = "./assets/sample_24bit.bmp";

int main(int argc, char *argv[]) {
    int                  ret   = 1;
    char                *image = NULL;
    bmp_FileHeader       file_header;
    bmp_BitmapInfoHeader bitmap_info_header;
    int                  error;

    (void)argc;
    (void)argv;

    error = bmp_read_bitmap(BMP_FILE, &file_header, &bitmap_info_header, &image);
    if (error != 0) {
        goto out;
    }

    bmp_PixelRGB24 *pixel = (bmp_PixelRGB24 *)image;

    test(pixel->blue == 0);
    test(pixel->green == 0);
    test(pixel->red == 255);

    ret = 0;

out:
    free(image);
    return ret;
}
