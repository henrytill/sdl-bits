///
/// Test for bmp_read_bitmap_v4() function.
///
/// This test reads a 32-bit bitmap file and checks that the first pixel is
/// semi-transparent red.
///
/// @see bmp_read_bitmap_v4()
///
#include <stddef.h>
#include <string.h>

#include "bmp.h"
#include "prelude.h"

int main(__attribute__((unused)) int argc, __attribute__((unused)) char* argv[]) {
  const char* const bmpFile = "./assets/test.bmp";
  bmp_FileHeader fileHeader = {0};
  bmp_V4Header v4Header = {0};
  SCOPED_PTR_char image = NULL;

  if (bmp_V4Read(bmpFile, &fileHeader, &v4Header, &image) != 0) {
    return EXIT_FAILURE;
  }

  bmp_Pixel32 expected = {
    .b = 255,
    .g = 0,
    .r = 0,
    .a = 127,
  };

  if (memcmp(&expected, (bmp_Pixel32*)image, sizeof(bmp_Pixel32)) != 0) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
