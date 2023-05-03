#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "macro.h"

enum {
  DWORDBITS = 32,
  DWORDBYTES = 4,
};

const uint16_t FILETYPE = 0x4D42;
const uint32_t BI_BITFIELDS = 0x0003;
const uint32_t LCS_WINDOWS_COLOR_SPACE = 0x57696E20;

DEFINE_TRIVIAL_CLEANUP_FUNC(FILE *, fclose);
#define _cleanup_FILE_ _cleanup_(fclosep)

size_t bmp_rowSize(uint16_t bitsPerPixel, int32_t width) {
  const double pixelBits = (double)bitsPerPixel * width;
  return (size_t)(ceil(pixelBits / DWORDBITS)) * DWORDBYTES;
}

int bmp_read(const char *file, struct bmp_FileHeader *fileHeader,
             struct bmp_InfoHeader *infoHeader, char **image) {
  int rc;
  size_t reads;
  uint32_t size;
  fpos_t pos;

  _cleanup_FILE_ FILE *fileHandle = fopen(file, "r");
  if (fileHandle == NULL)
    return -1;

  reads = fread(fileHeader, sizeof(*fileHeader), 1, fileHandle);
  if (reads != 1)
    return -1;

  rc = fgetpos(fileHandle, &pos);
  if (rc != 0)
    return -1;

  reads = fread(&size, sizeof(size), 1, fileHandle);
  if (reads != 1)
    return -1;
  if (size != BITMAPINFOHEADER)
    return -1;

  rc = fsetpos(fileHandle, &pos);
  if (rc != 0)
    return -1;

  reads = fread(infoHeader, sizeof(*infoHeader), 1, fileHandle);
  if (reads != 1)
    return -1;

  const uint32_t imageSize = infoHeader->imageSize;
  *image = calloc(imageSize, sizeof(**image));
  if (*image == NULL)
    return -1;

  reads = fread(*image, imageSize * sizeof(**image), 1, fileHandle);
  if (reads != 1) {
    free(*image);
    return -1;
  }

  return 0;
}

int bmp_v4read(const char *file, struct bmp_FileHeader *fileHeader,
               struct bmp_V4Header *v4Header, char **image) {
  int rc;
  size_t reads;
  uint32_t size;
  fpos_t pos;

  _cleanup_FILE_ FILE *fileHandle = fopen(file, "r");
  if (fileHandle == NULL)
    return -1;

  reads = fread(fileHeader, sizeof(*fileHeader), 1, fileHandle);
  if (reads != 1)
    return -1;

  rc = fgetpos(fileHandle, &pos);
  if (rc != 0)
    return -1;

  reads = fread(&size, sizeof(size), 1, fileHandle);
  if (reads != 1)
    return -1;
  if (size != BITMAPV4HEADER)
    return -1;

  rc = fsetpos(fileHandle, &pos);
  if (rc != 0)
    return -1;

  reads = fread(v4Header, sizeof(*v4Header), 1, fileHandle);
  if (reads != 1)
    return -1;

  const uint32_t imageSize = v4Header->imageSize;
  *image = calloc(imageSize, sizeof(**image));
  if (*image == NULL)
    return -1;

  reads = fread(*image, imageSize * sizeof(**image), 1, fileHandle);
  if (reads != 1) {
    free(*image);
    return -1;
  }

  return 0;
}

int bmp_v4write(const struct bmp_Pixel32 *buffer,
                size_t width, size_t height,
                const char *file) {
  size_t writes;

  if (buffer == NULL || file == NULL)
    return -1;
  if (width > INT32_MAX || height > INT32_MAX)
    return -1;

  const size_t imageSize = (width * height) * sizeof(struct bmp_Pixel32);
  if (imageSize > UINT32_MAX)
    return -1;

  const size_t offset = sizeof(struct bmp_FileHeader) + sizeof(struct bmp_V4Header);

  const size_t fileSize = offset + imageSize;
  if (fileSize > UINT32_MAX)
    return -1;

  struct bmp_FileHeader fileHeader = {
    .fileType = FILETYPE,
    .fileSize = (uint32_t)fileSize,
    .reserved1 = 0,
    .reserved2 = 0,
    .offset = (uint32_t)offset,
  };

  struct bmp_V4Header v4Header = {
    .size = BITMAPV4HEADER,
    .width = (int32_t)width,
    .height = (int32_t)height,
    .planes = 1,
    .bitsPerPixel = 32,
    .compression = BI_BITFIELDS,
    .imageSize = (uint32_t)imageSize,
    .horizontalResolution = 0,
    .verticalResolution = 0,
    .colors = 0,
    .importantColors = 0,
    .redMask = 0x00FF0000,
    .greenMask = 0x0000FF00,
    .blueMask = 0x000000FF,
    .alphaMask = 0xFF000000,
    .colorspaceType = LCS_WINDOWS_COLOR_SPACE,
    .colorspace = {0, 0, 0, 0, 0, 0, 0, 0, 0},
    .redGamma = 0,
    .greenGamma = 0,
    .blueGamma = 0,
  };

  _cleanup_FILE_ FILE *fileHandle = fopen(file, "wb");
  if (fileHandle == NULL)
    return -1;

  writes = fwrite(&fileHeader, sizeof(struct bmp_FileHeader), 1, fileHandle);
  if (writes != 1)
    return -1;

  writes = fwrite(&v4Header, sizeof(struct bmp_V4Header), 1, fileHandle);
  if (writes != 1)
    return -1;

  writes = fwrite(buffer, imageSize, 1, fileHandle);
  if (writes != 1)
    return -1;

  return 0;
}
