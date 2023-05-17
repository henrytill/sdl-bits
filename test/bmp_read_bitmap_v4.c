///
/// Test for bmp_read_bitmap_v4() function.
///
/// This test reads a 32-bit bitmap file and checks that the first pixel is
/// semi-transparent red.
///
/// @see bmp_read_bitmap_v4()
///
#include <stddef.h>
#include <stdlib.h>

#include "bmp.h"
#include "prelude.h"

int main(_unused_ int argc, _unused_ char* argv[]) {
  bmp_FileHeader fileHeader;
  bmp_V4Header v4Header;
  const char* const bmpFile = "./assets/test.bmp";
  _cleanup_str_ char* image = NULL;

  if (bmp_V4Read(bmpFile, &fileHeader, &v4Header, &image) != 0)
    return EXIT_FAILURE;

  bmp_Pixel32* pixel = (bmp_Pixel32*)image;
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
