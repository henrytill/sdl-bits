///
/// Test for bmp_read_bitmap_v4() function.
///
/// This test reads a 32-bit bitmap file and checks that the first pixel is
/// semi-transparent red.
///
/// @see bmp_v4_read()
///
#include <stddef.h>
#include <string.h>

#include "bmp.h"
#include "prelude.h"

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
    const char *const bmp_file = "./assets/test.bmp";
    bmp_file_header file_header = {0};
    bmp_v4_header v4_header = {0};
    SCOPED_PTR_char image = NULL;

    if (bmp_v4_read(bmp_file, &file_header, &v4_header, &image) != 0) {
        return EXIT_FAILURE;
    }

    bmp_pixel32 expected = {
        .b = 255,
        .g = 0,
        .r = 0,
        .a = 127,
    };

    if (memcmp(&expected, (bmp_pixel32 *)image, sizeof(bmp_pixel32)) != 0) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
