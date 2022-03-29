#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

// Integer constants used for calculating row size
enum {
    BITS_PER_DWORD  = 32,
    BYTES_PER_DWORD = 4,
};

// File header constants
static const uint16_t BITMAP_FILE_TYPE       = 0x4D42;
static const size_t   BITMAP_V4_OFFSET_BYTES = sizeof(bmp_FileHeader) + sizeof(bmp_BitmapV4Header);

// Bitmap v4 header constants
static const uint16_t             NUM_PLANES              = 1;
static const uint16_t             BITS_PER_PIXEL          = 32;
static const uint32_t             BI_BITFIELDS            = 0x0003;
static const uint32_t             ARGB32_RED_MASK         = 0x00FF0000;
static const uint32_t             ARGB32_GREEN_MASK       = 0x0000FF00;
static const uint32_t             ARGB32_BLUE_MASK        = 0x000000FF;
static const uint32_t             ARGB32_ALPHA_MASK       = 0xFF000000;
static const uint32_t             LCS_WINDOWS_COLOR_SPACE = 0x57696E20;
static const bmp_ColorSpaceTriple COLOR_SPACE_TRIPLE      = {0, 0, 0, 0, 0, 0, 0, 0, 0};

static const char *const MODE_READ  = "r";
static const char *const MODE_WRITE = "wb";

size_t bmp_row_size(uint16_t bits_per_pixel, int32_t width_pixels) {
    return (size_t)(ceil((double)bits_per_pixel * width_pixels / BITS_PER_DWORD)) * BYTES_PER_DWORD;
}

int bmp_write_bitmap_v4(const bmp_PixelARGB32 *target_buff,
                        size_t                 image_width_pixels,
                        size_t                 image_height_pixels,
                        const char            *file) {
    int    ret    = 1;
    FILE  *file_h = NULL;
    size_t writes;

    if (target_buff == NULL || file == NULL) {
        return ret;
    }

    if (image_width_pixels > INT32_MAX || image_height_pixels > INT32_MAX) {
        return ret;
    }

    const size_t image_size_bytes =
        (image_width_pixels * image_height_pixels) * sizeof(bmp_PixelARGB32);
    if (image_size_bytes > UINT32_MAX) {
        return ret;
    }

    const size_t file_size_bytes = BITMAP_V4_OFFSET_BYTES + image_size_bytes;
    if (file_size_bytes > UINT32_MAX) {
        return ret;
    }

    bmp_FileHeader file_header = {
        .type         = BITMAP_FILE_TYPE,
        .size_bytes   = (uint32_t)file_size_bytes,
        .reserved1    = 0,
        .reserved2    = 0,
        .offset_bytes = (uint32_t)BITMAP_V4_OFFSET_BYTES,
    };

    bmp_BitmapV4Header bitmap_v4_header = {
        .dib_header_size_bytes = BITMAPV4HEADER,
        .width_pixels          = (int32_t)image_width_pixels,
        .height_pixels         = (int32_t)image_height_pixels,
        .num_planes            = NUM_PLANES,
        .bits_per_pixel        = BITS_PER_PIXEL,
        .compression           = BI_BITFIELDS,
        .image_size_bytes      = (uint32_t)image_size_bytes,
        .x_resolution_ppm      = 0,
        .y_resolution_ppm      = 0,
        .num_colors            = 0,
        .num_important_colors  = 0,
        .red_mask              = ARGB32_RED_MASK,
        .green_mask            = ARGB32_GREEN_MASK,
        .blue_mask             = ARGB32_BLUE_MASK,
        .alpha_mask            = ARGB32_ALPHA_MASK,
        .color_space_type      = LCS_WINDOWS_COLOR_SPACE,
        .color_space_triple    = COLOR_SPACE_TRIPLE,
        .red_gamma             = 0,
        .green_gamma           = 0,
        .blue_gamma            = 0,
    };

    file_h = fopen(file, MODE_WRITE);
    if (file_h == NULL) {
        return ret;
    }

    writes = fwrite(&file_header, sizeof(bmp_FileHeader), 1, file_h);
    if (writes != 1) {
        goto out;
    }

    writes = fwrite(&bitmap_v4_header, sizeof(bmp_BitmapV4Header), 1, file_h);
    if (writes != 1) {
        goto out;
    }

    writes = fwrite(target_buff, image_size_bytes, 1, file_h);
    if (writes != 1) {
        goto out;
    }

    ret = 0;

out:
    fclose(file_h);
    return ret;
}

int bmp_read_bitmap(const char           *file,
                    bmp_FileHeader       *file_header_out,
                    bmp_BitmapInfoHeader *bitmap_info_header_out,
                    char                **image_out) {
    int      ret    = 1;
    FILE    *file_h = NULL;
    uint32_t dib_header_size_bytes;
    size_t   reads;
    int      error;
    fpos_t   pos;

    file_h = fopen(file, MODE_READ);
    if (file_h == NULL) {
        return ret;
    }

    reads = fread(file_header_out, sizeof(bmp_FileHeader), 1, file_h);
    if (reads != 1) {
        goto out;
    }

    error = fgetpos(file_h, &pos);
    if (error != 0) {
        goto out;
    }

    reads = fread(&dib_header_size_bytes, sizeof(uint32_t), 1, file_h);
    if (reads != 1) {
        goto out;
    }

    error = fsetpos(file_h, &pos);
    if (error != 0) {
        goto out;
    }

    if (dib_header_size_bytes != BITMAPINFOHEADER) {
        goto out;
    }

    reads = fread(bitmap_info_header_out, sizeof(bmp_BitmapInfoHeader), 1, file_h);
    if (reads != 1) {
        goto out;
    }

    const uint32_t image_size_bytes = bitmap_info_header_out->image_size_bytes;

    *image_out = calloc(image_size_bytes, sizeof(char));
    if (*image_out == NULL) {
        goto out;
    }

    reads = fread(*image_out, image_size_bytes * sizeof(char), 1, file_h);
    if (reads != 1) {
        goto out;
    }

    ret = 0;

out:
    fclose(file_h);
    return ret;
}

int bmp_read_bitmap_v4(const char         *file,
                       bmp_FileHeader     *file_header_out,
                       bmp_BitmapV4Header *bitmap_v4_header_out,
                       char              **image_out) {
    int      ret    = 1;
    FILE    *file_h = NULL;
    uint32_t dib_header_size_bytes;
    size_t   reads;
    fpos_t   pos;
    int      error;

    file_h = fopen(file, MODE_READ);
    if (file_h == NULL) {
        return ret;
    }

    reads = fread(file_header_out, sizeof(bmp_FileHeader), 1, file_h);
    if (reads != 1) {
        goto out;
    }

    error = fgetpos(file_h, &pos);
    if (error != 0) {
        goto out;
    }

    reads = fread(&dib_header_size_bytes, sizeof(uint32_t), 1, file_h);
    if (reads != 1) {
        goto out;
    }

    error = fsetpos(file_h, &pos);
    if (error != 0) {
        goto out;
    }

    if (dib_header_size_bytes != BITMAPV4HEADER) {
        goto out;
    }

    reads = fread(bitmap_v4_header_out, sizeof(bmp_BitmapV4Header), 1, file_h);
    if (reads != 1) {
        goto out;
    }

    const uint32_t image_size_bytes = bitmap_v4_header_out->image_size_bytes;

    *image_out = calloc(image_size_bytes, sizeof(char));
    if (*image_out == NULL) {
        goto out;
    }

    reads = fread(*image_out, image_size_bytes * sizeof(char), 1, file_h);
    if (reads != 1) {
        goto out;
    }

    ret = 0;

out:
    fclose(file_h);
    return ret;
}
