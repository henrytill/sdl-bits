#include <stdint.h>
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

struct util_buffer_s {
	uint32_t cap;
	uint32_t count;
	char    *data;
};

int
util_buffer_init(util_buffer_t **buff, uint32_t cap)
{
	/* Allocate buffer and assign to buff if successful */
	util_buffer_t *temp = (util_buffer_t *)malloc(sizeof(util_buffer_t));
	if (temp == NULL) {
		return 1;
	}
	*buff = temp;
	/* Allocate data and assign to data field if successful */
	char *data = (char *)calloc(cap, sizeof(char));
	if (data == NULL) {
		return 1;
	}
	(*buff)->data = data;
	/* Assign values to remaining fields */
	(*buff)->cap   = cap;
	(*buff)->count = 0;
	return 0;
}

int
util_buffer_deinit(util_buffer_t **buff)
{
	if ((*buff) == NULL) {
		return 0;
	}
	if ((*buff)->data != NULL) {
		free((*buff)->data);
	}
	(*buff)->cap   = 0;
	(*buff)->count = 0;
	(*buff)->data  = NULL;
	free(*buff);
	*buff = NULL;
	return 0;
}

uint32_t
util_buffer_cap(util_buffer_t *buff)
{
	if (buff == NULL) {
		return 0;
	}
	return buff->cap;
}

uint32_t
util_buffer_count(util_buffer_t *buff)
{
	if (buff == NULL) {
		return 0;
	}
	return buff->count;
}

static int
util_buffer_grow(util_buffer_t *buff, uint32_t cap_hint, uint32_t growth)
{
	char *data;

	if (cap_hint <= buff->cap) {
		return 0;
	}
	uint64_t new_cap = (uint64_t)buff->cap;
	while (cap_hint > new_cap) {
		new_cap *= growth;
	}
	if (new_cap > UINT32_MAX) {
		new_cap = UINT32_MAX;
	}
	/* Allocate new data and assign to data field if successful */
	data = (char *)realloc(buff->data, (size_t)new_cap * sizeof(char));
	if (data == NULL) {
		return 1;
	}
	buff->data = data;
	/* Update cap field */
	buff->cap = (uint32_t)new_cap;
	return 0;
}

int
util_buffer_push(util_buffer_t *buff, char item)
{
	if (buff == NULL || buff->data == NULL) {
		return 1;
	}
	if (buff->count == UINT32_MAX) {
		return 1;
	}
	uint32_t new_count = buff->count + 1;
	if (util_buffer_grow(buff, new_count, BUFFER_GROWTH_FACTOR) == 1) {
		return 1;
	}
	*(buff->data + buff->count) = item;
	buff->count                 = new_count;
	return 0;
}

int
util_buffer_set(util_buffer_t *buff, uint32_t index, char item)
{
	if (buff == NULL || buff->data == NULL) {
		return 1;
	}
	uint32_t new_count = index + 1;
	if (util_buffer_grow(buff, new_count, BUFFER_GROWTH_FACTOR) == 1) {
		return 1;
	}
	*(buff->data + index) = item;
	if (index >= buff->count) {
		buff->count = new_count;
	}
	return 0;
}

int
util_buffer_read(util_buffer_t *buff, uint32_t index, char *out)
{
	if (buff == NULL || buff->data == NULL) {
		return 1;
	}
	if (index >= buff->cap) {
		return 1;
	}
	*out = *(buff->data + index);
	return 0;
}
