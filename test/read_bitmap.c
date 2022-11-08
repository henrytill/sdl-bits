#include <stddef.h>
#include <stdlib.h>

#include "bmp.h"

static const char *const BMP_FILE = "./assets/sample_24bit.bmp";

int main(int argc, char *argv[])
{
  int ret = EXIT_FAILURE;
  char *image = NULL;
  struct bmp_Filehdr filehdr;
  struct bmp_Infohdr infohdr;

  (void)argc;
  (void)argv;

  if (bmp_read(BMP_FILE, &filehdr, &infohdr, &image) != 0)
    goto out;
  struct bmp_Pixel24 *pixel = (struct bmp_Pixel24 *)image;
  if (pixel->b != 0)
    goto out;
  if (pixel->g != 0)
    goto out;
  if (pixel->r != 255)
    goto out;
  ret = EXIT_SUCCESS;
out:
  free(image);
  return ret;
}
