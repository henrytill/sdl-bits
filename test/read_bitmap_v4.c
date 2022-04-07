#include <stddef.h>
#include <stdlib.h>

#include "test.h"
#include "bmp.h"

static const char *const BMP_FILE = "./assets/test.bmp";

int main(int argc, char *argv[]) {
    char *image = NULL;
    struct bmp_FileHeader file_header;
    struct bmp_BitmapV4Header bitmap_v4_header;
    int error;

    (void)argc;
    (void)argv;

    error = bmp_read_bitmap_v4(BMP_FILE, &file_header, &bitmap_v4_header, &image);
    if (error != 0) {
        goto out;
    }

    struct bmp_PixelARGB32 *pixel = (struct bmp_PixelARGB32 *)image;

    if (pixel->blue != 255) {
        print_failure(__FILE__, __LINE__);
        error = EXIT_FAILURE;
        goto out;
    }
    if (pixel->green != 0) {
        print_failure(__FILE__, __LINE__);
        error = EXIT_FAILURE;
        goto out;
    }
    if (pixel->red != 0) {
        print_failure(__FILE__, __LINE__);
        error = EXIT_FAILURE;
        goto out;
    }
    if (pixel->alpha != 127) {
        print_failure(__FILE__, __LINE__);
        error = EXIT_FAILURE;
        goto out;
    }
out:
    free(image);
    return error;
}
