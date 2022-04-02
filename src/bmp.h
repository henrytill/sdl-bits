#ifndef SDL_BITS_BMP_H
#define SDL_BITS_BMP_H

#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)

enum bmp_DIBHeaderSize {
    BITMAPCOREHEADER = 12,
    OS22XBITMAPHEADER = 64,
    BITMAPINFOHEADER = 40,
    BITMAPV2INFOHEADER = 52,
    BITMAPV3INFOHEADER = 56,
    BITMAPV4HEADER = 108,
    BITMAPV5HEADER = 124
};

struct bmp_FileHeader {
    uint16_t type;
    uint32_t size_bytes;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset_bytes;
};

struct bmp_BitmapInfoHeader {
    uint32_t dib_header_size_bytes;
    int32_t width_pixels;
    int32_t height_pixels;
    uint16_t num_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size_bytes;
    int32_t x_resolution_ppm;
    int32_t y_resolution_ppm;
    uint32_t num_colors;
    uint32_t num_important_colors;
};

struct bmp_ColorSpaceTriple {
    int32_t red_x;
    int32_t red_y;
    int32_t red_z;
    int32_t green_x;
    int32_t green_y;
    int32_t green_z;
    int32_t blue_x;
    int32_t blue_y;
    int32_t blue_z;
};

struct bmp_BitmapV4Header {
    uint32_t dib_header_size_bytes;
    int32_t width_pixels;
    int32_t height_pixels;
    uint16_t num_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size_bytes;
    int32_t x_resolution_ppm;
    int32_t y_resolution_ppm;
    uint32_t num_colors;
    uint32_t num_important_colors;
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t alpha_mask;
    uint32_t color_space_type;
    struct bmp_ColorSpaceTriple color_space_triple;
    uint32_t red_gamma;
    uint32_t green_gamma;
    uint32_t blue_gamma;
};

struct bmp_PixelRGB24 {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
};

struct bmp_PixelARGB32 {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
};

#pragma pack(pop)

size_t bmp_row_size(uint16_t bits_per_pixel, int32_t width_pixels);

int bmp_write_bitmap_v4(const struct bmp_PixelARGB32 *source_buff,
                        size_t width_pixels,
                        size_t height_pixels,
                        const char *file);

int bmp_read_bitmap(const char *file,
                    struct bmp_FileHeader *file_header_out,
                    struct bmp_BitmapInfoHeader *bitmap_info_header_out,
                    char **image_out);

int bmp_read_bitmap_v4(const char *file,
                       struct bmp_FileHeader *file_header_out,
                       struct bmp_BitmapV4Header *bitmap_v4_header_out,
                       char **image_out);

#endif /* SDL_BITS_BMP_H */
