/**
 * Test for bmp_read_bitmap_v4() function.
 *
 * This test reads a 32-bit bitmap file and checks that the first pixel is
 * semi-transparent red.
 *
 * @see bmp_read_bitmap_v4()
 */
#include <stddef.h>
#include <stdlib.h>

#include "bmp.h"
#include "prelude.h"

static const char *const BMP_FILE = "./assets/test.bmp";

int main(_unused_ int argc, _unused_ char *argv[]) {
  extern const char *const BMP_FILE;

  struct bmp_Filehdr filehdr;
  struct bmp_V4hdr v4hdr;

  _cleanup_str_ char *image = NULL;
  if (bmp_v4read(BMP_FILE, &filehdr, &v4hdr, &image) != 0)
    return EXIT_FAILURE;

  struct bmp_Pixel32 *pixel = (struct bmp_Pixel32 *)image;
  if (pixel->b != 255)
    return EXIT_FAILURE;
  if (pixel->g != 0)
    return EXIT_FAILURE;
  if (pixel->r != 0)
    return EXIT_FAILURE;
  if (pixel->a != 127)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
