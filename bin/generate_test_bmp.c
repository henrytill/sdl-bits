#include <stdlib.h>

#include "bmp.h"
#include "macro.h"

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

static const char* const bmpFile = "./assets/test.bmp";

int main(_unused_ int argc, _unused_ char* argv[]) {
  extern const struct bmp_Pixel32 buf[];
  extern const char* const bmpFile;

  return (bmp_v4write(buf, WIDTH, HEIGHT, bmpFile) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
