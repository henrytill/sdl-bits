///
/// Test for bmp_read() function.
///
/// This test reads a 24-bit bitmap file and checks that the first pixel is black.
///
/// @see bmp_read()
///
#include <stddef.h>
#include <stdlib.h>

#include "bmp.h"
#include "prelude.h"

int main(_unused_ int argc, _unused_ char *argv[]) {
  struct bmp_FileHeader fileHeader;
  struct bmp_InfoHeader infoHeader;
  const char *const bmpFile = "./assets/sample_24bit.bmp";

  _cleanup_str_ char *image = NULL;
  if (bmp_read(bmpFile, &fileHeader, &infoHeader, &image) != 0)
    return EXIT_FAILURE;

  struct bmp_Pixel24 *pixel = (struct bmp_Pixel24 *)image;
  if (pixel->blue != 0)
    return EXIT_FAILURE;
  if (pixel->green != 0)
    return EXIT_FAILURE;
  if (pixel->red != 255)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
