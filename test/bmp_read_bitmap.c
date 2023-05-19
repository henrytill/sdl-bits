///
/// Test for bmp_read() function.
///
/// This test reads a 24-bit bitmap file and checks that the first pixel is black.
///
/// @see bmp_read()
///
#include <stddef.h>
#include <string.h>

#include "bmp.h"
#include "prelude.h"

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[]) {
  const char *const bmpFile = "./assets/sample_24bit.bmp";
  bmp_FileHeader fileHeader = {0};
  bmp_InfoHeader infoHeader = {0};
  SCOPED_PTR_char image = NULL;

  if (bmp_read(bmpFile, &fileHeader, &infoHeader, &image) != 0) {
    return EXIT_FAILURE;
  }

  bmp_Pixel24 expected = {
    .b = 0,
    .g = 0,
    .r = 255,
  };

  if (memcmp(&expected, (bmp_Pixel24 *)image, sizeof(bmp_Pixel24)) != 0) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
