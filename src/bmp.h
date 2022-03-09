#ifndef SDL_BITS_BMP_H
#define SDL_BITS_BMP_H

#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)

typedef enum bmp_dib_header_size_e {
	BITMAPCOREHEADER   = 12,
	OS22XBITMAPHEADER  = 64,
	BITMAPINFOHEADER   = 40,
	BITMAPV2INFOHEADER = 52,
	BITMAPV3INFOHEADER = 56,
	BITMAPV4HEADER     = 108,
	BITMAPV5HEADER     = 124
} bmp_dib_header_size_t;

typedef struct bmp_file_header_s {
	// Header field
	uint16_t type;
	// File size in bytes
	uint32_t size;
	// Reserved
	uint16_t reserved1;
	// Reserved
	uint16_t reserved2;
	// Offset to image data in bytes
	uint32_t offset;
	// Size of DIB header
	uint32_t dib_header_size;
} bmp_file_header_t;

typedef struct bmp_bitmap_info_header_s {
	// Bitmap width in pixels
	int32_t width_px;
	// Bitmap height in pixels
	int32_t height_px;
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
	uint32_t important_colors;
} bmp_bitmap_info_header_t;

typedef struct bmp_color_space_triple_s {
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
} bmp_color_space_triple_t;

typedef struct bmp_bitmap_v4_header_s {
	// Bitmap width in pixels
	int32_t width_px;
	// Bitmap height in pixels
	int32_t height_px;
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
	uint32_t important_colors;
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
	bmp_color_space_triple_t color_space_triple;
	// Red gamma
	uint32_t red_gamma;
	// Green gamma
	uint32_t green_gamma;
	// Blue gamma
	uint32_t blue_gamma;
} bmp_bitmap_v4_header_t;

typedef struct bmp_pixel_RGB24_s {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
} bmp_pixel_RGB24_t;

typedef struct bmp_pixel_ARGB32_s {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t alpha;
} bmp_pixel_ARGB32_t;

size_t
bmp_row_size(uint16_t bits_per_pixel, int32_t width_px);

#pragma pack(pop)

#endif // SDL_BITS_BMP_H