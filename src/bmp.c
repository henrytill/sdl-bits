#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

enum {
	SUCCESS = 0,
	FAILURE = 1,
};

enum {
	DWORDBITS = 32,
	DWORDBYTES = 4,
};

const uint16_t FILETYPE = 0x4D42;
const uint32_t BI_BITFIELDS = 0x0003;
const uint32_t LCS_WINDOWS_COLOR_SPACE = 0x57696E20;

size_t
bmp_rowsize(uint16_t bpp, int32_t width)
{
	double pixelbits = (double)bpp * width;
	return (size_t)(ceil(pixelbits / DWORDBITS)) * DWORDBYTES;
}

int
bmp_read(const char *file, struct bmp_Filehdr *filehdr,
	struct bmp_Infohdr *infohdr, char **image)
{
	int rc = FAILURE;
	uint32_t size;
	size_t reads;
	fpos_t pos;

	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		return rc;
	}

	reads = fread(filehdr, sizeof(struct bmp_Filehdr), 1, fp);
	if (reads != 1) {
		goto out;
	}

	rc = fgetpos(fp, &pos);
	if (rc != SUCCESS) {
		goto out;
	}

	reads = fread(&size, sizeof(uint32_t), 1, fp);
	if (reads != 1) {
		rc = FAILURE;
		goto out;
	}

	rc = fsetpos(fp, &pos);
	if (rc != SUCCESS) {
		goto out;
	}

	if (size != BITMAPINFOHEADER) {
		rc = FAILURE;
		goto out;
	}

	reads = fread(infohdr, sizeof(struct bmp_Infohdr), 1, fp);
	if (reads != 1) {
		rc = FAILURE;
		goto out;
	}

	const uint32_t imagesize = infohdr->imagesize;

	*image = calloc(imagesize, sizeof(char));
	if (*image == NULL) {
		rc = FAILURE;
		goto out;
	}

	reads = fread(*image, imagesize * sizeof(char), 1, fp);
	if (reads != 1) {
		rc = FAILURE;
		goto out;
	}

	assert(rc == SUCCESS);
out:
	fclose(fp);
	return rc;
}

int
bmp_v4read(const char *file, struct bmp_Filehdr *filehdr,
	struct bmp_V4hdr *v4hdr, char **image)
{
	int rc = FAILURE;
	uint32_t size;
	size_t reads;
	fpos_t pos;

	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		return rc;
	}

	reads = fread(filehdr, sizeof(struct bmp_Filehdr), 1, fp);
	if (reads != 1) {
		goto out;
	}

	rc = fgetpos(fp, &pos);
	if (rc != SUCCESS) {
		goto out;
	}

	reads = fread(&size, sizeof(uint32_t), 1, fp);
	if (reads != 1) {
		rc = FAILURE;
		goto out;
	}

	rc = fsetpos(fp, &pos);
	if (rc != SUCCESS) {
		goto out;
	}

	if (size != BITMAPV4HEADER) {
		rc = FAILURE;
		goto out;
	}

	reads = fread(v4hdr, sizeof(struct bmp_V4hdr), 1, fp);
	if (reads != 1) {
		rc = FAILURE;
		goto out;
	}

	const uint32_t imagesize = v4hdr->imagesize;

	*image = calloc(imagesize, sizeof(char));
	if (*image == NULL) {
		rc = FAILURE;
		goto out;
	}

	reads = fread(*image, imagesize * sizeof(char), 1, fp);
	if (reads != 1) {
		rc = FAILURE;
		goto out;
	}

	assert(rc == SUCCESS);
out:
	fclose(fp);
	return rc;
}

int
bmp_v4write(const struct bmp_Pixel32 *buf, size_t width, size_t height,
	const char *file)
{
	int rc = FAILURE;
	size_t writes;
	const size_t offset =
		sizeof(struct bmp_Filehdr) + sizeof(struct bmp_V4hdr);

	if (buf == NULL || file == NULL) {
		return rc;
	}

	if (width > INT32_MAX || height > INT32_MAX) {
		return rc;
	}

	const size_t imagesize = (width * height) * sizeof(struct bmp_Pixel32);
	if (imagesize > UINT32_MAX) {
		return rc;
	}

	const size_t filesize = offset + imagesize;
	if (filesize > UINT32_MAX) {
		return rc;
	}

	struct bmp_Filehdr file_header = {
		.filetype = FILETYPE,
		.filesize = (uint32_t)filesize,
		.reserved1 = 0,
		.reserved2 = 0,
		.offset = (uint32_t)offset,
	};

	struct bmp_V4hdr bitmap_v4_header = {
		.size = BITMAPV4HEADER,
		.width = (int32_t)width,
		.height = (int32_t)height,
		.planes = 1,
		.bpp = 32,
		.compression = BI_BITFIELDS,
		.imagesize = (uint32_t)imagesize,
		.hres = 0,
		.vres = 0,
		.colors = 0,
		.impcolors = 0,
		.rmask = 0x00FF0000,
		.gmask = 0x0000FF00,
		.bmask = 0x000000FF,
		.amask = 0xFF000000,
		.colorspacetype = LCS_WINDOWS_COLOR_SPACE,
		.colorspace = {0, 0, 0, 0, 0, 0, 0, 0, 0},
		.rgamma = 0,
		.ggamma = 0,
		.bgamma = 0,
	};

	FILE *fp = fopen(file, "wb");
	if (fp == NULL) {
		return rc;
	}

	writes = fwrite(&file_header, sizeof(struct bmp_Filehdr), 1, fp);
	if (writes != 1) {
		goto out;
	}

	writes = fwrite(&bitmap_v4_header, sizeof(struct bmp_V4hdr), 1, fp);
	if (writes != 1) {
		goto out;
	}

	writes = fwrite(buf, imagesize, 1, fp);
	if (writes != 1) {
		goto out;
	}

	rc = SUCCESS;
out:
	fclose(fp);
	return rc;
}
