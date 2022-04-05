#ifndef SDL_BITS_UTIL_H
#define SDL_BITS_UTIL_H

#include <stdint.h>

#define util_log_sdl_error(category)                                                               \
    do {                                                                                           \
        const char *sdl_err = SDL_GetError();                                                      \
        if (strlen(sdl_err) != 0) {                                                                \
            SDL_LogError((category), "%s:%d: %s", __FILE__, __LINE__, sdl_err);                    \
        } else {                                                                                   \
            SDL_LogError((category), "%s:%d", __FILE__, __LINE__);                                 \
        }                                                                                          \
    } while (0)

/* Ref: http://locklessinc.com/articles/sat_arithmetic/ */
uint32_t util_uint32_sat_sub(uint32_t x, uint32_t y);

struct util_Buffer;

int util_buffer_init(struct util_Buffer **buff, uint32_t cap);

int util_buffer_deinit(struct util_Buffer **buff);

uint32_t util_buffer_cap(struct util_Buffer *buff);

uint32_t util_buffer_count(struct util_Buffer *buff);

int util_buffer_push(struct util_Buffer *buff, char item);

int util_buffer_set(struct util_Buffer *buff, uint32_t index, char item);

int util_buffer_read(struct util_Buffer *buff, uint32_t index, char *out);

struct util_Buffer2d;

int util_buffer2d_init(struct util_Buffer2d **buff, uint32_t x_cap, uint32_t y_cap);

int util_buffer2d_deinit(struct util_Buffer2d **buff);

uint32_t util_buffer2d_x_cap(struct util_Buffer2d *buff);

uint32_t util_buffer2d_y_cap(struct util_Buffer2d *buff);

int util_buffer2d_set(struct util_Buffer2d *buff, uint32_t x_index, uint32_t y_index, char item);

int util_buffer2d_read(struct util_Buffer2d *buff, uint32_t x_index, uint32_t y_index, char *out);

#endif /* SDL_BITS_UTIL_H */
