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
  const char *const bmp_file = "./assets/sample_24bit.bmp";
  bmp_file_header file_header = {0};
  bmp_info_header info_header = {0};
  SCOPED_PTR_char image = NULL;

  if (bmp_read(bmp_file, &file_header, &info_header, &image) != 0) {
    return EXIT_FAILURE;
  }

  bmp_pixel24 expected = {
    .b = 0,
    .g = 0,
    .r = 255,
  };

  if (memcmp(&expected, (bmp_pixel24 *)image, sizeof(bmp_pixel24)) != 0) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
