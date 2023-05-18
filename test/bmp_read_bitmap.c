///
/// Test for bmp_Read() function.
///
/// This test reads a 24-bit bitmap file and checks that the first pixel is black.
///
/// @see bmp_Read()
///
#include <stddef.h>

#include "bmp.h"
#include "prelude.h"

int main(__attribute__((unused)) int argc, __attribute__((unused)) char* argv[]) {
  bmp_FileHeader fileHeader;
  bmp_InfoHeader infoHeader;
  const char* const bmpFile = "./assets/sample_24bit.bmp";
  SCOPED_char image = NULL;

  if (bmp_Read(bmpFile, &fileHeader, &infoHeader, &image) != 0) {
    return EXIT_FAILURE;
  }

  bmp_Pixel24* pixel = (bmp_Pixel24*)image;
  if (pixel->b != 0) {
    return EXIT_FAILURE;
  }
  if (pixel->g != 0) {
    return EXIT_FAILURE;
  }
  if (pixel->r != 255) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
