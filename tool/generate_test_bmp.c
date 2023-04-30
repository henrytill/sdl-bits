#include <stdlib.h>

#include "bmp.h"

enum { SUCCESS = 0 };

enum {
  WIDTH = 4,
  HEIGHT = 2,
};

static const struct bmp_Pixel32 buf[] = {
  {0xFF, 0x00, 0x00, 0x7F},
  {0x00, 0xFF, 0x00, 0x7F},
  {0x00, 0x00, 0xFF, 0x7F},
  {0xFF, 0xFF, 0xFF, 0x7F},
  {0xFF, 0x00, 0x00, 0xFF},
  {0x00, 0xFF, 0x00, 0xFF},
  {0x00, 0x00, 0xFF, 0xFF},
  {0xFF, 0xFF, 0xFF, 0xFF},
};

static const char *const bmpfile = "./test.bmp";

int main(int argc, char *argv[]) {
  extern const struct bmp_Pixel32 buf[];
  extern const char *const bmpfile;

  (void)argc;
  (void)argv;

  return (bmp_v4write(buf, WIDTH, HEIGHT, bmpfile) == SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
