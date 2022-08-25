#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "bmp.h"

enum { SUCCESS = 0 };

enum {
  WIDTH = 10,
  HEIGHT = 20,
  CODESZ = 94, /* ('~' - '!') + 1 */
};

static const char *const FONTFILE = "./ucs-fonts/10x20.bdf";
static const char *const BMPFILE = "./10x20.bmp";

static const struct bmp_Pixel32 WHITE = {0xFF, 0xFF, 0xFF, 0x00};
static const struct bmp_Pixel32 BLACK = {0x00, 0x00, 0x00, 0xFF};

static char **allocimage(size_t height, size_t width);
static void freeimage(char **image, size_t height);

/* pos = 0 is MSB */
static char getbit(unsigned char c, size_t pos) {
  if (pos >= CHAR_BIT)
    return 0;
  return (c >> (CHAR_BIT + ~pos)) & 1; /* Also: c & (1 << (CHAR_BIT + ~pos)); */
}

static char **allocimage(size_t height, size_t width) {
  char **ret = NULL;

  if ((ret = calloc(height, sizeof(char *))) == NULL)
    return ret;
  for (size_t i = 0; i < height; ++i)
    if ((ret[i] = calloc(width, sizeof(char))) == NULL) {
      freeimage(ret, i);
      return NULL;
    }
  return ret;
}

static void freeimage(char **image, size_t height) {
  if (image == NULL)
    return;
  for (size_t i = 0; i < height; ++i)
    free(image[i]);
  free(image);
}

/* https://freetype.org/freetype2/docs/reference/ft2-basic_types.html#ft_bitmap */
static void renderchar(FT_GlyphSlot slot, char **target, size_t offset) {
  unsigned char *buffer = slot->bitmap.buffer;
  size_t rows = slot->bitmap.rows;
  size_t width = slot->bitmap.width;
  size_t pitch = (size_t)abs(slot->bitmap.pitch);
  char bit;

  for (size_t y = 0, p = 0; y < rows; ++y, p += pitch)
    for (size_t i = 0; i < pitch; ++i)
      for (size_t j = 0, x; j < CHAR_BIT; ++j) {
        bit = getbit(buffer[p + i], j);
        x = j + (i * CHAR_BIT);
        if (x < width) target[y][x + (offset * width)] = bit;
      }
}

#ifdef DRAW_IMAGE
static void drawimage(char **image, size_t width, size_t height) {
  for (size_t y = 0; y < height; ++y) {
    printf("%2zd|", y);
    for (size_t x = 0; x < width; ++x)
      putchar(image[y][x] ? '*' : ' ');
    printf("|\n");
  }
}
#else
static inline void drawimage(char **image, size_t width, size_t height) {
  (void)image;
  (void)width;
  (void)height;
}
#endif

int main(int argc, char *argv[]) {
  int ret = EXIT_FAILURE;
  int rc;
  char **image = NULL;
  FT_Library ftlib = NULL;
  FT_Face face = NULL;
  FT_GlyphSlot slot = NULL;
  const size_t width = WIDTH * CODESZ;
  const size_t height = HEIGHT;
  char code[CODESZ];
  struct bmp_Pixel32 *buf;

  (void)argc;
  (void)argv;

  for (int i = 0; i < CODESZ; ++i)
    code[i] = (char)(i + '!');

  if ((image = allocimage(height, width)) == NULL) {
    fprintf(stderr, "allocimage failed.");
    return EXIT_FAILURE;
  }

  if ((rc = FT_Init_FreeType(&ftlib)) != SUCCESS) {
    fprintf(stderr, "FT_Init_FreeType failed.  Error code: %d", rc);
    goto out0;
  }

  if ((rc = FT_New_Face(ftlib, FONTFILE, 0, &face)) != SUCCESS) {
    fprintf(stderr, "FT_New_Face failed.  Error code: %d", rc);
    goto out1;
  }

  if ((rc = FT_Set_Pixel_Sizes(face, WIDTH, HEIGHT)) != SUCCESS) {
    fprintf(stderr, "FT_Set_Pixel_Sizes failed.  Error code: %d", rc);
    goto out1;
  }

  for (size_t i = 0; i < CODESZ; ++i) {
    if ((rc = FT_Load_Char(face, (FT_ULong)code[i], FT_LOAD_NO_SCALE | FT_LOAD_MONOCHROME)) != SUCCESS) {
      fprintf(stderr, "FT_Load_Char failed.  Error code: %d", rc);
      goto out1;
    }
    slot = face->glyph;
    if ((rc = FT_Render_Glyph(slot, FT_RENDER_MODE_MONO)) != SUCCESS) {
      fprintf(stderr, "FT_Render_Glyph failed.  Error code: %d", rc);
      goto out1;
    }
    if (slot->format != FT_GLYPH_FORMAT_BITMAP) {
      fprintf(stderr, "format is not FL_GLYPH_FORMAT_BITMAP");
      goto out1;
    }
    if (slot->bitmap.pixel_mode != FT_PIXEL_MODE_MONO) {
      fprintf(stderr, "pixel_mode is not FL_PIXEL_MODE_MONO");
      goto out1;
    }
    renderchar(slot, image, i);
  }

  drawimage(image, width, height);

  if ((buf = calloc(width * height, sizeof(struct bmp_Pixel32))) == NULL)
    goto out2;

  for (size_t y = height, i = 0; y-- > 0;)
    for (size_t x = 0; x < width; ++x, ++i)
      buf[i] = image[y][x] ? BLACK : WHITE;

  if (bmp_v4write(buf, width, height, BMPFILE) != SUCCESS)
    goto out3;

  ret = EXIT_SUCCESS;
out3:
  free(buf);
out2:
  FT_Done_Face(face);
out1:
  FT_Done_FreeType(ftlib);
out0:
  freeimage(image, height);
  return ret;
}
