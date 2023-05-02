/**
 * Test for bmp_read() function.
 *
 * This test reads a 24-bit bitmap file and checks that the first pixel is
 * black.
 *
 * @see bmp_read()
 */
#include <stddef.h>
#include <stdlib.h>

#include "bmp.h"
#include "prelude.h"

static const char *const BMP_FILE = "./assets/sample_24bit.bmp";

int main(_unused_ int argc, _unused_ char *argv[]) {
  extern const char *const BMP_FILE;

  struct bmp_Filehdr filehdr;
  struct bmp_Infohdr infohdr;

  _cleanup_str_ char *image = NULL;
  if (bmp_read(BMP_FILE, &filehdr, &infohdr, &image) != 0)
    return EXIT_FAILURE;

  struct bmp_Pixel24 *pixel = (struct bmp_Pixel24 *)image;
  if (pixel->b != 0)
    return EXIT_FAILURE;
  if (pixel->g != 0)
    return EXIT_FAILURE;
  if (pixel->r != 255)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
