#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "macro.h"

enum {
  DWORDBITS = 32,
  DWORDBYTES = 4,
};

const uint16_t bmp_FILETYPE = 0x4D42;
const uint32_t bmp_BI_BITFIELDS = 0x0003;
const uint32_t bmp_LCS_WINDOWS_COLOR_SPACE = 0x57696E20;

DEFINE_TRIVIAL_CLEANUP_FUNC(FILE*, fclose)
#define SCOPED_FILE __attribute__((cleanup(fclosep))) FILE*

size_t bmp_RowSize(uint16_t bitsPerPixel, int32_t width) {
  const double pixelBits = (double)bitsPerPixel * width;
  return (size_t)(ceil(pixelBits / DWORDBITS)) * DWORDBYTES;
}

int bmp_Read(const char* file, bmp_FileHeader* fileHeader, bmp_InfoHeader* infoHeader, char** image) {
  SCOPED_FILE fileHandle = fopen(file, "r");
  if (fileHandle == NULL) {
    return -1;
  }

  size_t reads = fread(fileHeader, sizeof(*fileHeader), 1, fileHandle);
  if (reads != 1) {
    return -1;
  }

  fpos_t pos;
  int rc = fgetpos(fileHandle, &pos);
  if (rc != 0) {
    return -1;
  }

  uint32_t size = 0;
  reads = fread(&size, sizeof(size), 1, fileHandle);
  if (reads != 1) {
    return -1;
  }
  if (size != BITMAPINFOHEADER) {
    return -1;
  }

  rc = fsetpos(fileHandle, &pos);
  if (rc != 0) {
    return -1;
  }

  reads = fread(infoHeader, sizeof(*infoHeader), 1, fileHandle);
  if (reads != 1) {
    return -1;
  }

  const uint32_t imageSize = infoHeader->imageSize;
  *image = calloc(imageSize, sizeof(**image));
  if (*image == NULL) {
    return -1;
  }

  reads = fread(*image, imageSize * sizeof(**image), 1, fileHandle);
  if (reads != 1) {
    free(*image);
    return -1;
  }

  return 0;
}

int bmp_V4Read(const char* file, bmp_FileHeader* fileHeader, bmp_V4Header* v4Header, char** image) {
  SCOPED_FILE fileHandle = fopen(file, "r");
  if (fileHandle == NULL) {
    return -1;
  }

  size_t reads = fread(fileHeader, sizeof(*fileHeader), 1, fileHandle);
  if (reads != 1) {
    return -1;
  }

  fpos_t pos;
  int rc = fgetpos(fileHandle, &pos);
  if (rc != 0) {
    return -1;
  }

  uint32_t size = 0;
  reads = fread(&size, sizeof(size), 1, fileHandle);
  if (reads != 1) {
    return -1;
  }
  if (size != BITMAPV4HEADER) {
    return -1;
  }

  rc = fsetpos(fileHandle, &pos);
  if (rc != 0) {
    return -1;
  }

  reads = fread(v4Header, sizeof(*v4Header), 1, fileHandle);
  if (reads != 1) {
    return -1;
  }

  const uint32_t imageSize = v4Header->imageSize;
  *image = calloc(imageSize, sizeof(**image));
  if (*image == NULL) {
    return -1;
  }

  reads = fread(*image, imageSize * sizeof(**image), 1, fileHandle);
  if (reads != 1) {
    free(*image);
    return -1;
  }

  return 0;
}

int bmp_V4Write(const bmp_Pixel32* buffer, size_t width, size_t height, const char* file) {
  if (buffer == NULL || file == NULL) {
    return -1;
  }
  if (width > INT32_MAX || height > INT32_MAX) {
    return -1;
  }

  const size_t imageSize = (width * height) * sizeof(bmp_Pixel32);
  if (imageSize > UINT32_MAX) {
    return -1;
  }

  const size_t offset = sizeof(bmp_FileHeader) + sizeof(bmp_V4Header);

  const size_t fileSize = offset + imageSize;
  if (fileSize > UINT32_MAX) {
    return -1;
  }

  bmp_FileHeader fileHeader = {
    .fileType = bmp_FILETYPE,
    .fileSize = (uint32_t)fileSize,
    .reserved1 = 0,
    .reserved2 = 0,
    .offset = (uint32_t)offset,
  };

  bmp_V4Header v4Header = {
    .size = BITMAPV4HEADER,
    .width = (int32_t)width,
    .height = (int32_t)height,
    .planes = 1,
    .bitsPerPixel = 32,
    .compression = bmp_BI_BITFIELDS,
    .imageSize = (uint32_t)imageSize,
    .hRes = 0,
    .vRes = 0,
    .colors = 0,
    .impColors = 0,
    .rMask = 0x00FF0000,
    .gMask = 0x0000FF00,
    .bMask = 0x000000FF,
    .aMask = 0xFF000000,
    .colorspaceType = bmp_LCS_WINDOWS_COLOR_SPACE,
    .colorspace = {0, 0, 0, 0, 0, 0, 0, 0, 0},
    .rGamma = 0,
    .gGamma = 0,
    .bGamma = 0,
  };

  SCOPED_FILE fileHandle = fopen(file, "wb");
  if (fileHandle == NULL) {
    return -1;
  }

  size_t writes = fwrite(&fileHeader, sizeof(bmp_FileHeader), 1, fileHandle);
  if (writes != 1) {
    return -1;
  }

  writes = fwrite(&v4Header, sizeof(bmp_V4Header), 1, fileHandle);
  if (writes != 1) {
    return -1;
  }

  writes = fwrite(buffer, imageSize, 1, fileHandle);
  if (writes != 1) {
    return -1;
  }

  return 0;
}
