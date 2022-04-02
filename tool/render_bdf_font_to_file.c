#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "bmp.h"

#define print_error(error)                                                                         \
    do {                                                                                           \
        fprintf(stderr, "ERROR: %s:%d: %d", __FILE__, __LINE__, error);                            \
    } while (0)

enum {
    EXPECTED_CHAR_BIT = 8,
    FONT_WIDTH_PIXELS = 10,
    FONT_HEIGHT_PIXELS = 20,
    CHAR_CODES_SIZE = 94, /* ('~' - '!') + 1 */
};

static const char *const FONT_FILE = "./ucs-fonts/10x20.bdf";
static const char *const BMP_FILE = "./10x20.bmp";

static const struct bmp_PixelARGB32 WHITE = {0xFF, 0xFF, 0xFF, 0x00};
static const struct bmp_PixelARGB32 BLACK = {0x00, 0x00, 0x00, 0xFF};

/* pos = 0 is MSB */
static unsigned char get_bit(unsigned char source, size_t pos);

static int alloc_image(unsigned char ***image, size_t height, size_t width);

static void free_image(unsigned char ***image, size_t height);

static void render_char(FT_GlyphSlot slot, unsigned char **target, size_t offset);

#ifdef DRAW_IMAGE
static void draw_image(unsigned char **image, size_t image_width, size_t image_height);
#else
static inline void draw_image(unsigned char **image, size_t image_width, size_t image_height);
#endif

static unsigned char get_bit(unsigned char source, size_t pos) {
    assert(CHAR_BIT == EXPECTED_CHAR_BIT);
    if (pos >= CHAR_BIT) {
        return 0;
    }
    /* Also: source & (1 << (CHAR_BIT + ~pos)); */
    return (source >> (CHAR_BIT + ~pos)) & 1;
}

static int alloc_image(unsigned char ***image, size_t height, size_t width) {
    *image = calloc(height, sizeof(unsigned char *));
    if (*image == NULL) {
        return 1;
    }
    for (size_t i = 0; i < height; i++) {
        *(*image + i) = calloc(width, sizeof(unsigned char));
        if (*(*image + i) == NULL) {
            return 1;
        }
    }
    return 0;
}

void free_image(unsigned char ***image, size_t height) {
    if (image == NULL) {
        return;
    }
    for (size_t i = 0; i < height; i++) {
        free(*(*image + i));
    }
    free(*image);
    *image = NULL;
}

static void render_char(FT_GlyphSlot slot, unsigned char **target, size_t offset) {
    unsigned char *buffer = slot->bitmap.buffer;
    unsigned int rows = (unsigned int)slot->bitmap.rows;
    unsigned int width = (unsigned int)slot->bitmap.width;
    unsigned int pitch = (unsigned int)abs(slot->bitmap.pitch);
    unsigned char datum;

    for (size_t y = 0, p = 0; y < rows; y++, p += pitch) {
        for (size_t i = 0; i < pitch; i++) {
            for (size_t j = 0, x; j < CHAR_BIT; j++) {
                datum = get_bit(*(buffer + p + i), j);

                x = j + (i * CHAR_BIT);
                if (x < width) {
                    *(*(target + y) + x + (offset * width)) = datum;
                }
            }
        }
    }
}

#ifdef DRAW_IMAGE
static void draw_image(unsigned char **image, size_t image_width, size_t image_height) {
    for (size_t y = 0; y < image_height; y++) {
        printf("%2zd|", y);
        for (size_t x = 0; x < image_width; x++) {
            putchar(image[y][x] ? '*' : ' ');
        }
        printf("|\n");
    }
}
#else
static inline void draw_image(unsigned char **image, size_t image_width, size_t image_height) {
    (void)image;
    (void)image_width;
    (void)image_height;
}
#endif

int main(int argc, char *argv[]) {
    FT_Library library = NULL;
    FT_Face face = NULL;
    FT_GlyphSlot slot = NULL;
    unsigned char **image = NULL;
    struct bmp_PixelARGB32 *target_buff = NULL;
    const size_t image_width = FONT_WIDTH_PIXELS * CHAR_CODES_SIZE;
    const size_t image_height = FONT_HEIGHT_PIXELS;
    char char_codes[CHAR_CODES_SIZE];
    int error;

    (void)argc;
    (void)argv;

    for (size_t i = 0; i < CHAR_CODES_SIZE; i++) {
        char_codes[i] = (char)(i + '!');
    }

    error = alloc_image(&image, image_height, image_width);
    if (error != 0) {
        print_error(error);
        goto out;
    }

    error = FT_Init_FreeType(&library);
    if (error != 0) {
        print_error(error);
        goto out;
    }

    error = FT_New_Face(library, FONT_FILE, 0, &face);
    if (error != 0) {
        print_error(error);
        goto out;
    }

    error = FT_Set_Pixel_Sizes(face, FONT_WIDTH_PIXELS, FONT_HEIGHT_PIXELS);
    if (error != 0) {
        print_error(error);
        goto out;
    }

    for (size_t i = 0; i < CHAR_CODES_SIZE; i++) {
        error = FT_Load_Char(face, (FT_ULong)char_codes[i], FT_LOAD_NO_SCALE | FT_LOAD_MONOCHROME);
        if (error != 0) {
            print_error(error);
            goto out;
        }

        slot = face->glyph;
        error = FT_Render_Glyph(slot, FT_RENDER_MODE_MONO);
        if (error != 0) {
            print_error(error);
            goto out;
        }
        if (slot->format != FT_GLYPH_FORMAT_BITMAP) {
            fprintf(stderr, "format is not FL_GLYPH_FORMAT_BITMAP");
            error = 1;
            goto out;
        }
        if (slot->bitmap.pixel_mode != FT_PIXEL_MODE_MONO) {
            fprintf(stderr, "pixel_mode is not FL_PIXEL_MODE_MONO");
            error = 1;
            goto out;
        }

        render_char(slot, image, i);
    }

    draw_image(image, image_width, image_height);

    target_buff = calloc(image_width * image_height, sizeof(struct bmp_PixelARGB32));
    if (target_buff == NULL) {
        error = 1;
        goto out;
    }

    for (size_t y = image_height, i = 0; y-- > 0;) {
        for (size_t x = 0; x < image_width; x++, i++) {
            *(target_buff + i) = *(*(image + y) + x) ? BLACK : WHITE;
        }
    }

    error = bmp_write_bitmap_v4(target_buff, image_width, image_height, BMP_FILE);

out:
    free(target_buff);
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    free_image(&image, image_height);
    return error;
}
