#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

enum {
  DWORDBITS = 32,
  DWORDBYTES = 4,
};

const uint16_t FILETYPE = 0x4D42;
const uint32_t BI_BITFIELDS = 0x0003;
const uint32_t LCS_WINDOWS_COLOR_SPACE = 0x57696E20;

size_t bmp_rowSize(uint16_t bitsPerPixel, int32_t width) {
  const double pixelBits = (double)bitsPerPixel * width;
  return (size_t)(ceil(pixelBits / DWORDBITS)) * DWORDBYTES;
}

int bmp_read(const char *file, struct bmp_FileHeader *fileHeader,
             struct bmp_InfoHeader *infoHeader, char **image) {
  int ret = -1;
  int rc;
  size_t reads;
  FILE *fileHandle = NULL;
  uint32_t size;
  fpos_t pos;

  fileHandle = fopen(file, "r");
  if (fileHandle == NULL)
    return ret;

  reads = fread(fileHeader, sizeof(*fileHeader), 1, fileHandle);
  if (reads != 1)
    goto out;

  rc = fgetpos(fileHandle, &pos);
  if (rc != 0)
    goto out;

  reads = fread(&size, sizeof(size), 1, fileHandle);
  if (reads != 1)
    goto out;
  if (size != BITMAPINFOHEADER)
    goto out;

  rc = fsetpos(fileHandle, &pos);
  if (rc != 0)
    goto out;

  reads = fread(infoHeader, sizeof(*infoHeader), 1, fileHandle);
  if (reads != 1)
    goto out;

  const uint32_t imageSize = infoHeader->imageSize;
  *image = calloc(imageSize, sizeof(**image));
  if (*image == NULL)
    goto out;

  reads = fread(*image, imageSize * sizeof(**image), 1, fileHandle);
  if (reads != 1) {
    free(*image);
    goto out;
  }

  ret = 0;
out:
  fclose(fileHandle);
  return ret;
}

int bmp_v4read(const char *file, struct bmp_FileHeader *fileHeader,
               struct bmp_V4Header *v4Header, char **image) {
  int ret = -1;
  int rc;
  size_t reads;
  FILE *fileHandle = NULL;
  uint32_t size;
  fpos_t pos;

  fileHandle = fopen(file, "r");
  if (fileHandle == NULL)
    return ret;

  reads = fread(fileHeader, sizeof(*fileHeader), 1, fileHandle);
  if (reads != 1)
    goto out;

  rc = fgetpos(fileHandle, &pos);
  if (rc != 0)
    goto out;

  reads = fread(&size, sizeof(size), 1, fileHandle);
  if (reads != 1)
    goto out;
  if (size != BITMAPV4HEADER)
    goto out;

  rc = fsetpos(fileHandle, &pos);
  if (rc != 0)
    goto out;

  reads = fread(v4Header, sizeof(*v4Header), 1, fileHandle);
  if (reads != 1)
    goto out;

  const uint32_t imageSize = v4Header->imageSize;
  *image = calloc(imageSize, sizeof(**image));
  if (*image == NULL)
    goto out;

  reads = fread(*image, imageSize * sizeof(**image), 1, fileHandle);
  if (reads != 1) {
    free(*image);
    goto out;
  }

  ret = 0;
out:
  fclose(fileHandle);
  return ret;
}

int bmp_v4write(const struct bmp_Pixel32 *buffer,
                size_t width, size_t height,
                const char *file) {
  int ret = -1;
  size_t writes;
  FILE *fileHandle = NULL;

  if (buffer == NULL || file == NULL)
    return ret;
  if (width > INT32_MAX || height > INT32_MAX)
    return ret;

  const size_t imageSize = (width * height) * sizeof(struct bmp_Pixel32);
  if (imageSize > UINT32_MAX)
    return ret;

  const size_t offset = sizeof(struct bmp_FileHeader) + sizeof(struct bmp_V4Header);

  const size_t fileSize = offset + imageSize;
  if (fileSize > UINT32_MAX)
    return ret;

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

  fileHandle = fopen(file, "wb");
  if (fileHandle == NULL)
    return ret;

  writes = fwrite(&fileHeader, sizeof(struct bmp_FileHeader), 1, fileHandle);
  if (writes != 1)
    goto out;

  writes = fwrite(&v4Header, sizeof(struct bmp_V4Header), 1, fileHandle);
  if (writes != 1)
    goto out;

  writes = fwrite(buffer, imageSize, 1, fileHandle);
  if (writes != 1)
    goto out;

  ret = 0;
out:
  fclose(fileHandle);
  return ret;
}
