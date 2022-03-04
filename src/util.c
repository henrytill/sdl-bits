#include <stdlib.h>

#include "util.h"

uint32_t
util_uint32_sat_sub(uint32_t x, uint32_t y)
{
	uint32_t ret;

	ret = x - y;
	ret &= -(ret <= x);
	return ret;
}

int
util_buffer_init(util_buffer_t *buff, size_t size)
{
	buff->size = size;
	buff->curr = 0;
	buff->data = (char *)calloc(size, sizeof(char));
	return 0;
}

int
util_buffer_deinit(util_buffer_t *buff)
{
	if (buff->data != NULL) {
		free(buff->data);
	}
	buff->size = 0;
	buff->curr = 0;
	buff->data = NULL;
	return 0;
}

int
util_buffer_push(util_buffer_t *buff, char item)
{
	if (buff->data == NULL) {
		return 1;
	}
	size_t size = buff->size;
	if (buff->curr == size) {
		size *= BUFFER_GROWTH_FACTOR;
		/* Overflow check */
		if (size <= buff->size) {
			return 1;
		}
		buff->size = size;
		buff->data = (char *)realloc(buff->data, size * sizeof(char));
	}
	*(buff->data + buff->curr) = item;
	buff->curr += 1;
	return 0;
}

int
util_buffer_set(util_buffer_t *buff, size_t index, char item)
{
	if (buff->data == NULL) {
		return 1;
	}
	if (index >= buff->size) {
		return 1;
	}
	*(buff->data + index) = item;
	return 0;
}

int
util_buffer_read(util_buffer_t *buff, size_t index, char *out)
{
	if (buff->data == NULL) {
		return 1;
	}
	if (index >= buff->size) {
		return 1;
	}
	*out = *(buff->data + index);
	return 0;
}
