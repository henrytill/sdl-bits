#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "bmp.h"
#include "macro.h"

enum {
  WIDTH = 10,
  HEIGHT = 20,
  CODE_SIZE = 94, // ('~' - '!') + 1
};

static const char* const fontFile = "./assets/ucs-fonts/10x20.bdf";
static const char* const bmpFile = "./assets/10x20.bmp";

static const bmp_Pixel32 white = {0xFF, 0xFF, 0xFF, 0x00};
static const bmp_Pixel32 black = {0x00, 0x00, 0x00, 0xFF};

static char** AllocImage(size_t height, size_t width);
static void FreeImage(char** image, size_t height);

// pos = 0 is MSB
static char GetBit(unsigned char c, size_t pos) {
  if (pos >= CHAR_BIT) {
    return 0;
  }
  return (char)((c >> (CHAR_BIT + ~pos)) & 1); // Also: c & (1 << (CHAR_BIT + ~pos));
}

static char** AllocImage(size_t height, size_t width) {
  char** ret = calloc(height, sizeof(*ret));
  if (ret == NULL) {
    return ret;
  }
  for (size_t i = 0; i < height; ++i) {
    ret[i] = calloc(width, sizeof(**ret));
    if (ret[i] == NULL) {
      FreeImage(ret, i);
      return NULL;
    }
  }
  return ret;
}

static void FreeImage(char** image, size_t height) {
  if (image == NULL) {
    return;
  }
  for (size_t i = 0; i < height; ++i) {
    free(image[i]);
  }
  free(image);
}

// https://freetype.org/freetype2/docs/reference/ft2-basic_types.html#ft_bitmap
static void RenderChar(FT_GlyphSlot slot, char** target, size_t offset) {
  unsigned char* buffer = slot->bitmap.buffer;
  size_t rows = (size_t)slot->bitmap.rows;
  size_t width = (size_t)slot->bitmap.width;
  size_t pitch = (size_t)abs(slot->bitmap.pitch);
  char bit = 0;

  for (size_t y = 0, p = 0; y < rows; ++y, p += pitch) {
    for (size_t i = 0; i < pitch; ++i) {
      for (size_t j = 0, x; j < CHAR_BIT; ++j) {
        bit = GetBit(buffer[p + i], j);
        x = j + (i * CHAR_BIT);
        if (x < width) {
          target[y][x + (offset * width)] = bit;
        }
      }
    }
  }
}

#ifdef DRAW_IMAGE
static void DrawImage(char** image, size_t width, size_t height) {
  for (size_t y = 0; y < height; ++y) {
    printf("%2zd|", y);
    for (size_t x = 0; x < width; ++x)
      putchar(image[y][x] ? '*' : ' ');
    printf("|\n");
  }
}
#else
static inline void DrawImage(__attribute__((unused)) char** image,
                             __attribute__((unused)) size_t width,
                             __attribute__((unused)) size_t height) {}
#endif

static void DestroyBuffer(bmp_Pixel32* buffer) { free(buffer); }

DEFINE_TRIVIAL_CLEANUP_FUNC(FT_Library, FT_Done_FreeType)
DEFINE_TRIVIAL_CLEANUP_FUNC(FT_Face, FT_Done_Face)
DEFINE_TRIVIAL_CLEANUP_FUNC(bmp_Pixel32*, DestroyBuffer)
#define SCOPED_FT_Library      __attribute__((cleanup(FT_Done_FreeTypep))) FT_Library
#define SCOPED_FT_Face         __attribute__((cleanup(FT_Done_Facep))) FT_Face
#define SCOPED_PTR_bmp_Pixel32 __attribute__((cleanup(DestroyBufferp))) bmp_Pixel32*

int main(void) {
  extern const char* const fontFile;
  extern const char* const bmpFile;
  extern const bmp_Pixel32 white;
  extern const bmp_Pixel32 black;

  char code[CODE_SIZE] = {0};
  for (int i = 0; i < CODE_SIZE; ++i) {
    code[i] = (char)(i + '!');
  }

  const size_t width = WIDTH * CODE_SIZE;
  const size_t height = HEIGHT;
  char** image = AllocImage(height, width);
  if (image == NULL) {
    fprintf(stderr, "AllocImage failed.");
    return EXIT_FAILURE;
  }

  SCOPED_FT_Library lib = NULL;
  int rc = FT_Init_FreeType(&lib);
  if (rc != 0) {
    fprintf(stderr, "FT_Init_FreeType failed.  Error code: %d", rc);
    FreeImage(image, height);
    return EXIT_FAILURE;
  }

  SCOPED_FT_Face face = NULL;
  rc = FT_New_Face(lib, fontFile, 0, &face);
  if (rc != 0) {
    fprintf(stderr, "FT_New_Face failed.  Error code: %d", rc);
    FreeImage(image, height);
    return EXIT_FAILURE;
  }

  rc = FT_Set_Pixel_Sizes(face, WIDTH, HEIGHT);
  if (rc != 0) {
    fprintf(stderr, "FT_Set_Pixel_Sizes failed.  Error code: %d", rc);
    FreeImage(image, height);
    return EXIT_FAILURE;
  }

  FT_GlyphSlot slot = NULL;
  for (size_t i = 0; i < CODE_SIZE; ++i) {
    rc = FT_Load_Char(face, (FT_ULong)code[i], FT_LOAD_NO_SCALE | FT_LOAD_MONOCHROME);
    if (rc != 0) {
      fprintf(stderr, "FT_Load_Char failed.  Error code: %d", rc);
      FreeImage(image, height);
      return EXIT_FAILURE;
    }
    slot = face->glyph;

    rc = FT_Render_Glyph(slot, FT_RENDER_MODE_MONO);
    if (rc != 0) {
      fprintf(stderr, "FT_Render_Glyph failed.  Error code: %d", rc);
      FreeImage(image, height);
      return EXIT_FAILURE;
    }
    if (slot->format != FT_GLYPH_FORMAT_BITMAP) {
      fprintf(stderr, "format is not FL_GLYPH_FORMAT_BITMAP");
      FreeImage(image, height);
      return EXIT_FAILURE;
    }
    if (slot->bitmap.pixel_mode != FT_PIXEL_MODE_MONO) {
      fprintf(stderr, "pixel_mode is not FL_PIXEL_MODE_MONO");
      FreeImage(image, height);
      return EXIT_FAILURE;
    }

    RenderChar(slot, image, i);
  }

  DrawImage(image, width, height);

  SCOPED_PTR_bmp_Pixel32 buffer = calloc(width * height, sizeof(bmp_Pixel32));
  if (buffer == NULL) {
    FreeImage(image, height);
    return EXIT_FAILURE;
  }

  for (size_t y = height, i = 0; y-- > 0;) {
    for (size_t x = 0; x < width; ++x, ++i) {
      buffer[i] = image[y][x] ? black : white;
    }
  }

  rc = bmp_V4Write(buffer, width, height, bmpFile);
  if (rc != 0) {
    FreeImage(image, height);
    return EXIT_FAILURE;
  }

  FreeImage(image, height);
  return EXIT_SUCCESS;
}
