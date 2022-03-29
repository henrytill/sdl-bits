#ifndef SDL_BITS_BMP_H
#define SDL_BITS_BMP_H

#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)

typedef enum bmp_DIBHeaderSize {
    BITMAPCOREHEADER   = 12,
    OS22XBITMAPHEADER  = 64,
    BITMAPINFOHEADER   = 40,
    BITMAPV2INFOHEADER = 52,
    BITMAPV3INFOHEADER = 56,
    BITMAPV4HEADER     = 108,
    BITMAPV5HEADER     = 124
} bmp_DIBHeaderSize;

typedef struct bmp_FileHeader {
    // Header field
    uint16_t type;
    // File size in bytes
    uint32_t size_bytes;
    // Reserved
    uint16_t reserved1;
    // Reserved
    uint16_t reserved2;
    // Offset to image data in bytes
    uint32_t offset_bytes;
} bmp_FileHeader;

typedef struct bmp_BitmapInfoHeader {
    // DIB header size in bytes
    uint32_t dib_header_size_bytes;
    // Bitmap width in pixels
    int32_t width_pixels;
    // Bitmap height in pixels
    int32_t height_pixels;
    // Number of color planes
    uint16_t num_planes;
    // Bits per pixel
    uint16_t bits_per_pixel;
    // Compression method
    uint32_t compression;
    // Image size in bytes
    uint32_t image_size_bytes;
    // Horizontal resolution of the image in pixels per meter
    int32_t x_resolution_ppm;
    // Vertical resolution of the image in pixels per meter
    int32_t y_resolution_ppm;
    // Number of colors in the color palette
    uint32_t num_colors;
    // Number of important colors used
    uint32_t num_important_colors;
} bmp_BitmapInfoHeader;

typedef struct bmp_ColorSpaceTriple {
    // X coordinate of red endpoint
    int32_t red_x;
    // Y coordinate of red endpoint
    int32_t red_y;
    // Z coordinate of red endpoint
    int32_t red_z;
    // X coordinate of green endpoint
    int32_t green_x;
    // Y coordinate of green endpoint
    int32_t green_y;
    // Z coordinate of green endpoint
    int32_t green_z;
    // X coordinate of blue endpoint
    int32_t blue_x;
    // Y coordinate of blue endpoint
    int32_t blue_y;
    // Z coordinate of blue endpoint
    int32_t blue_z;
} bmp_ColorSpaceTriple;

typedef struct bmp_BitmapV4Header {
    // DIB header size in bytes
    uint32_t dib_header_size_bytes;
    // Bitmap width in pixels
    int32_t width_pixels;
    // Bitmap height in pixels
    int32_t height_pixels;
    // Number of color planes
    uint16_t num_planes;
    // Bits per pixel
    uint16_t bits_per_pixel;
    // Compression method
    uint32_t compression;
    // Image size in bytes
    uint32_t image_size_bytes;
    // Horizontal resolution of the image in pixels per meter
    int32_t x_resolution_ppm;
    // Vertical resolution of the image in pixels per meter
    int32_t y_resolution_ppm;
    // Number of colors in the color palette
    uint32_t num_colors;
    // Number of important colors used
    uint32_t num_important_colors;
    // Red channel bit mask
    uint32_t red_mask;
    // Green channel bit mask
    uint32_t green_mask;
    // Blue channel bit mask
    uint32_t blue_mask;
    // Alpha channel bit mask
    uint32_t alpha_mask;
    // Color space type
    uint32_t color_space_type;
    // Color space triple
    bmp_ColorSpaceTriple color_space_triple;
    // Red gamma
    uint32_t red_gamma;
    // Green gamma
    uint32_t green_gamma;
    // Blue gamma
    uint32_t blue_gamma;
} bmp_BitmapV4Header;

typedef struct bmp_PixelRGB24 {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} bmp_PixelRGB24;

typedef struct bmp_PixelARGB32 {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} bmp_PixelARGB32;

#pragma pack(pop)

size_t bmp_row_size(uint16_t bits_per_pixel, int32_t width_pixels);

int bmp_write_bitmap_v4(const bmp_PixelARGB32 *target_buff,
                        size_t                 image_width_pixels,
                        size_t                 image_height_pixels,
                        const char            *file);

int bmp_read_bitmap(const char           *file,
                    bmp_FileHeader       *file_header_out,
                    bmp_BitmapInfoHeader *bitmap_info_header_out,
                    char                **image_out);

int bmp_read_bitmap_v4(const char         *file,
                       bmp_FileHeader     *file_header_out,
                       bmp_BitmapV4Header *bitmap_v4_header_out,
                       char              **image_out);

#endif // SDL_BITS_BMP_H
