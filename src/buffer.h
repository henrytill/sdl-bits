#ifndef SDL_BITS_BUFFER_H
#define SDL_BITS_BUFFER_H

#include <stdint.h>

struct Buffer;

int buffer_init(struct Buffer **buff, uint32_t cap);

int buffer_deinit(struct Buffer **buff);

uint32_t buffer_cap(struct Buffer *buff);

uint32_t buffer_count(struct Buffer *buff);

int buffer_push(struct Buffer *buff, char item);

int buffer_set(struct Buffer *buff, uint32_t index, char item);

int buffer_read(struct Buffer *buff, uint32_t index, char *out);

struct Buffer2d;

int buffer2d_init(struct Buffer2d **buff, uint32_t x_cap, uint32_t y_cap);

int buffer2d_deinit(struct Buffer2d **buff);

uint32_t buffer2d_x_cap(struct Buffer2d *buff);

uint32_t buffer2d_y_cap(struct Buffer2d *buff);

int buffer2d_set(struct Buffer2d *buff, uint32_t x_index, uint32_t y_index, char item);

int buffer2d_read(struct Buffer2d *buff, uint32_t x_index, uint32_t y_index, char *out);

#endif /* SDL_BITS_BUFFER_H */
