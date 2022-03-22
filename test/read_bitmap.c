#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "test.h"

static char *const BMP_FILE = "./assets/sample_24bit.bmp";

int main(int argc, char *argv[]) {
    int                      ret   = 1;
    char                    *image = NULL;
    bmp_file_header_t        file_header;
    bmp_bitmap_info_header_t bitmap_info_header;
    int                      error;

    (void)argc;
    (void)argv;

    error = bmp_read_bitmap(BMP_FILE, &file_header, &bitmap_info_header, &image);
    if (error != 0) {
        goto cleanup;
    }

    bmp_pixel_RGB24_t *pixel = (bmp_pixel_RGB24_t *)image;

    TEST(pixel->blue == 0);
    TEST(pixel->green == 0);
    TEST(pixel->red == 255);

    ret = 0;

cleanup:
    free(image);
    return ret;
}
