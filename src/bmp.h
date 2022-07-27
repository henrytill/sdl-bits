#ifndef SDL_BITS_BMP_H
#define SDL_BITS_BMP_H

#include <stddef.h>
#include <stdint.h>

extern const uint16_t FILETYPE;
extern const uint32_t BI_BITFIELDS;
extern const uint32_t LCS_WINDOWS_COLOR_SPACE;

#pragma pack(push, 1)

enum Headersz {
	BITMAPCOREHEADER = 12,
	OS22XBITMAPHEADER = 64,
	BITMAPINFOHEADER = 40,
	BITMAPV2INFOHEADER = 52,
	BITMAPV3INFOHEADER = 56,
	BITMAPV4HEADER = 108,
	BITMAPV5HEADER = 124
};

struct bmp_Filehdr {
	uint16_t filetype;
	uint32_t filesize;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t offset;
};

struct bmp_Infohdr {
	uint32_t size;        /* DIB Header size (bytes) */
	int32_t width;        /* Image width (pixels) */
	int32_t height;       /* Image height (pixels) */
	uint16_t planes;      /* Number of planes */
	uint16_t bpp;         /* Bits per pixel */
	uint32_t compression; /* Compression mode */
	uint32_t imagesize;   /* Image size (bytes) */
	int32_t hres;         /* Horizontal resolution (pixels per meter) */
	int32_t vres;         /* Vertical resolution (pixels per meter) */
	uint32_t colors;      /* Used colors */
	uint32_t impcolors;   /* Important colors */
};

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
};

struct bmp_V4hdr {
	uint32_t size;        /* DIB Header Size (bytes) */
	int32_t width;        /* Image width (pixels) */
	int32_t height;       /* Image height (pixels) */
	uint16_t planes;      /* Number of planes */
	uint16_t bpp;         /* Bits per pixel */
	uint32_t compression; /* Compression mode */
	uint32_t imagesize;   /* Image size (bytes) */
	int32_t hres;         /* Horizontal resolution (pixels per meter) */
	int32_t vres;         /* Vertical resolution (pixels per meter) */
	uint32_t colors;      /* Used colors */
	uint32_t impcolors;   /* Important colors */
	uint32_t rmask;
	uint32_t gmask;
	uint32_t bmask;
	uint32_t amask;
	uint32_t colorspacetype;
	struct bmp_Colorspace colorspace;
	uint32_t rgamma;
	uint32_t ggamma;
	uint32_t bgamma;
};

struct bmp_Pixel24 {
	uint8_t b;
	uint8_t g;
	uint8_t r;
};

struct bmp_Pixel32 {
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
};

#pragma pack(pop)

size_t bmp_rowsize(uint16_t bpp, int32_t width);

int bmp_read(const char *file, struct bmp_Filehdr *filehdr, struct bmp_Infohdr *infohdr, char **image);

int bmp_v4read(const char *file, struct bmp_Filehdr *filehdr, struct bmp_V4hdr *v4hdr, char **image);

int bmp_v4write(const struct bmp_Pixel32 *buf, size_t width, size_t height, const char *file);

#endif /* SDL_BITS_BMP_H */
