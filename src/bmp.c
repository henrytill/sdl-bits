#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

enum {
    BITS_PER_DWORD  = 32,
    BYTES_PER_DWORD = 4,
    NUM_PLANES      = 1,
    BITS_PER_PIXEL  = 32
};

static const uint16_t BITMAP_FILE_TYPE        = 0x4D42;
static const uint32_t BI_BITFIELDS            = 0x0003;
static const uint32_t ARGB24_RED_MASK         = 0x00FF0000;
static const uint32_t ARGB24_GREEN_MASK       = 0x0000FF00;
static const uint32_t ARGB24_BLUE_MASK        = 0x000000FF;
static const uint32_t ARGB24_ALPHA_MASK       = 0xFF000000;
static const uint32_t LCS_WINDOWS_COLOR_SPACE = 0x57696E20;

static char *const MODE_READ  = "r";
static char *const MODE_WRITE = "wb";

size_t bmp_row_size(uint16_t bits_per_pixel, int32_t width_pixels) {
    double bits;
    double width;

    bits  = (double)bits_per_pixel;
    width = (double)width_pixels;
    return (size_t)(ceil((bits * width) / BITS_PER_DWORD)) * BYTES_PER_DWORD;
}

int bmp_write_bitmap_v4(const bmp_PixelARGB32 *target_buff,
                        size_t                 image_width_pixels,
                        size_t                 image_height_pixels,
                        char                  *file) {
    int                  ret                = 1;
    FILE                *file_h             = NULL;
    bmp_ColorSpaceTriple color_space_triple = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    bmp_BitmapV4Header   bitmap_v4_header;
    bmp_FileHeader       file_header;
    size_t               image_size_bytes;
    size_t               file_size_bytes;
    size_t               writes;

    assert(bmp_bitmap_v4_offset < UINT32_MAX);

    if (target_buff == NULL) {
        return ret;
    }

    if (file == NULL) {
        return ret;
    }

    if (image_width_pixels > INT32_MAX || image_height_pixels > INT32_MAX) {
        return ret;
    }

    image_size_bytes = (image_width_pixels * image_height_pixels) * sizeof(bmp_PixelARGB32);
    if (image_size_bytes > UINT32_MAX) {
        return ret;
    }

    file_size_bytes = bmp_bitmap_v4_offset + image_size_bytes;
    if (file_size_bytes > UINT32_MAX) {
        return ret;
    }

    bitmap_v4_header.dib_header_size_bytes = BITMAPV4HEADER;
    bitmap_v4_header.width_pixels          = (int32_t)image_width_pixels;
    bitmap_v4_header.height_pixels         = (int32_t)image_height_pixels;
    bitmap_v4_header.num_planes            = NUM_PLANES;
    bitmap_v4_header.bits_per_pixel        = BITS_PER_PIXEL;
    bitmap_v4_header.compression           = BI_BITFIELDS;
    bitmap_v4_header.image_size_bytes      = (uint32_t)image_size_bytes;
    bitmap_v4_header.x_resolution_ppm      = 0;
    bitmap_v4_header.y_resolution_ppm      = 0;
    bitmap_v4_header.num_colors            = 0;
    bitmap_v4_header.num_important_colors  = 0;
    bitmap_v4_header.red_mask              = ARGB24_RED_MASK;
    bitmap_v4_header.green_mask            = ARGB24_GREEN_MASK;
    bitmap_v4_header.blue_mask             = ARGB24_BLUE_MASK;
    bitmap_v4_header.alpha_mask            = ARGB24_ALPHA_MASK;
    bitmap_v4_header.color_space_type      = LCS_WINDOWS_COLOR_SPACE;
    bitmap_v4_header.color_space_triple    = color_space_triple;
    bitmap_v4_header.red_gamma             = 0;
    bitmap_v4_header.green_gamma           = 0;
    bitmap_v4_header.blue_gamma            = 0;

    file_header.type         = BITMAP_FILE_TYPE;
    file_header.size_bytes   = (uint32_t)file_size_bytes;
    file_header.reserved1    = 0;
    file_header.reserved2    = 0;
    file_header.offset_bytes = (uint32_t)bmp_bitmap_v4_offset;

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

int bmp_read_bitmap(char                 *file,
                    bmp_FileHeader       *file_header_out,
                    bmp_BitmapInfoHeader *bitmap_info_header_out,
                    char                **image_out) {
    int      ret    = 1;
    FILE    *file_h = NULL;
    uint32_t dib_header_size_bytes;
    uint32_t image_size_bytes;
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

    image_size_bytes = bitmap_info_header_out->image_size_bytes;

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

int bmp_read_bitmap_v4(char               *file,
                       bmp_FileHeader     *file_header_out,
                       bmp_BitmapV4Header *bitmap_v4_header_out,
                       char              **image_out) {
    int      ret    = 1;
    FILE    *file_h = NULL;
    uint32_t dib_header_size_bytes;
    uint32_t image_size_bytes;
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

    if (dib_header_size_bytes != BITMAPV4HEADER) {
        goto out;
    }

    reads = fread(bitmap_v4_header_out, sizeof(bmp_BitmapV4Header), 1, file_h);
    if (reads != 1) {
        goto out;
    }

    image_size_bytes = bitmap_v4_header_out->image_size_bytes;

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
