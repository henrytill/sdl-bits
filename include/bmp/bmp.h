#pragma once

#include <stddef.h>
#include <stdint.h>

#include "macro.h"

extern const uint16_t FILETYPE;
extern const uint32_t BI_BITFIELDS;
extern const uint32_t LCS_WINDOWS_COLOR_SPACE;

enum HeaderSize {
  BITMAPCOREHEADER = 12,
  OS22XBITMAPHEADER = 64,
  BITMAPINFOHEADER = 40,
  BITMAPV2INFOHEADER = 52,
  BITMAPV3INFOHEADER = 56,
  BITMAPV4HEADER = 108,
  BITMAPV5HEADER = 124
};

struct bmp_FileHeader {
  uint16_t fileType;
  uint32_t fileSize;
  uint16_t reserved1;
  uint16_t reserved2;
  uint32_t offset;
} _packed_;

struct bmp_InfoHeader {
  uint32_t size;                // DIB Header size (bytes)
  int32_t width;                // Image width (pixels)
  int32_t height;               // Image height (pixels)
  uint16_t planes;              // Number of planes
  uint16_t bitsPerPixel;        // Bits per pixel
  uint32_t compression;         // Compression mode
  uint32_t imageSize;           // Image size (bytes)
  int32_t horizontalResolution; // Horizontal resolution (pixels per meter)
  int32_t vres;                 // Vertical resolution (pixels per meter)
  uint32_t colors;              // Used colors
  uint32_t importantColors;     // Important colors
} _packed_;

struct bmp_Colorspace {
  int32_t rx;
  int32_t ry;
  int32_t rz;
  int32_t gx;
  int32_t gy;
  int32_t gz;
  int32_t bx;
  int32_t by;
  int32_t bz;
} _packed_;

struct bmp_V4Header {
  uint32_t size;                // DIB Header Size (bytes)
  int32_t width;                // Image width (pixels)
  int32_t height;               // Image height (pixels)
  uint16_t planes;              // Number of planes
  uint16_t bitsPerPixel;        // Bits per pixel
  uint32_t compression;         // Compression mode
  uint32_t imageSize;           // Image size (bytes)
  int32_t horizontalResolution; // Horizontal resolution (pixels per meter)
  int32_t verticalResolution;   // Vertical resolution (pixels per meter)
  uint32_t colors;              // Used colors
  uint32_t importantColors;     // Important colors
  uint32_t redMask;
  uint32_t greenMask;
  uint32_t blueMask;
  uint32_t alphaMask;
  uint32_t colorspaceType;
  struct bmp_Colorspace colorspace;
  uint32_t redGamma;
  uint32_t greenGamma;
  uint32_t blueGamma;
} _packed_;

struct bmp_Pixel24 {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
} _packed_;

struct bmp_Pixel32 {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
  uint8_t alpha;
} _packed_;

///
/// Calculate the number of bytes per row.
///
/// @param bitsPerPixel Bits per pixel.
/// @param width Image width.
/// @return Number of bytes per row.
///
size_t bmp_rowSize(uint16_t bitsPerPixel, int32_t width);

///
/// Read a BMP file.
///
/// @param file Path to the BMP file.
/// @param fileHeader The file header structure to be filled.
/// @param infoHeader The info header structure to be filled.
/// @param image The image data to be filled.
/// @return 0 on success, -1 on error.
///
int bmp_read(const char *file, struct bmp_FileHeader *fileHeader, struct bmp_InfoHeader *infoHeader, char **image);

///
/// Reads a BMP file with a V4 header.
///
/// @param file Path to the BMP file.
/// @param fileHeader The file header structure to be filled.
/// @param v4hdr The V4 header structure to be filled.
/// @param image The image data to be filled.
/// @return 0 on success, -1 on error.
///
int bmp_v4read(const char *file, struct bmp_FileHeader *fileHeader, struct bmp_V4Header *v4hdr, char **image);

///
/// Write a BMP file with a V4 header.
///
/// @param buf The image data.
/// @param width Image width in pixels.
/// @param height Image height in pixels.
/// @param file Path to the BMP file
///
int bmp_v4write(const struct bmp_Pixel32 *buf, size_t width, size_t height, const char *file);
