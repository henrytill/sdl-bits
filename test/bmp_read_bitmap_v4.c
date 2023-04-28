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

static const char *const BMP_FILE = "./assets/test.bmp";

int main(int argc, char *argv[]) {
  int ret = EXIT_FAILURE;
  char *image = NULL;
  struct bmp_Filehdr filehdr;
  struct bmp_V4hdr v4hdr;

  (void)argc;
  (void)argv;

  if (bmp_v4read(BMP_FILE, &filehdr, &v4hdr, &image) != 0)
    goto out;
  struct bmp_Pixel32 *pixel = (struct bmp_Pixel32 *)image;
  if (pixel->b != 255)
    goto out;
  if (pixel->g != 0)
    goto out;
  if (pixel->r != 0)
    goto out;
  if (pixel->a != 127)
    goto out;
  ret = EXIT_SUCCESS;
out:
  free(image);
  return ret;
}
