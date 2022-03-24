#include <stddef.h>
#include <stdint.h>

#include "test.h"
#include "util.h"

// Some helper macros

#define checked_init(buff, size) test(util_buffer_init(buff, size) == 0)

#define checked_deinit(buff) test(util_buffer_deinit(buff) == 0)

#define checked_push(buff, item) test(util_buffer_push(buff, item) == 0)

#define checked_read(buff, index, expected)                                                        \
    do {                                                                                           \
        char out;                                                                                  \
        test(util_buffer_read(buff, index, &out) == 0);                                            \
        test(out == (expected));                                                                   \
    } while (0)

#define failed_read(buff, index)                                                                   \
    do {                                                                                           \
        char out;                                                                                  \
        test(util_buffer_read(buff, index, &out) == 1);                                            \
    } while (0)

#define checked_set(buff, index, item) test(util_buffer_set(buff, index, item) == 0)

#define check_cap(buff, expected) test(util_buffer_cap(buff) == (expected))

#define check_count(buff, expected) test(util_buffer_count(buff) == (expected))

int main(int argc, char *argv[]) {
    util_Buffer *buffer = NULL;

    (void)argc;
    (void)argv;

    checked_init(&buffer, 3);
    check_cap(buffer, 3);
    check_count(buffer, 0);

    checked_push(buffer, 0);
    checked_push(buffer, 1);
    checked_push(buffer, 2);
    checked_read(buffer, 0, 0);
    checked_read(buffer, 1, 1);
    checked_read(buffer, 2, 2);
    failed_read(buffer, 3);
    check_cap(buffer, 3);
    check_count(buffer, 3);

    checked_push(buffer, 3);
    checked_read(buffer, 0, 0);
    checked_read(buffer, 1, 1);
    checked_read(buffer, 2, 2);
    checked_read(buffer, 3, 3);
    failed_read(buffer, 4);
    check_cap(buffer, 6);
    check_count(buffer, 4);

    checked_set(buffer, 3, 42);
    checked_read(buffer, 0, 0);
    checked_read(buffer, 1, 1);
    checked_read(buffer, 2, 2);
    checked_read(buffer, 3, 42);
    check_cap(buffer, 6);
    check_count(buffer, 4);

    checked_deinit(&buffer);
    test(buffer == NULL);
    check_cap(buffer, 0);
    check_count(buffer, 0);

    checked_deinit(&buffer);
    checked_init(&buffer, 3);
    checked_set(buffer, 0, 42);
    checked_read(buffer, 0, 42);
    check_cap(buffer, 3);
    check_count(buffer, 1);

    checked_deinit(&buffer);
    checked_init(&buffer, 3);
    checked_set(buffer, 2, 42);
    checked_read(buffer, 2, 42);
    check_cap(buffer, 3);
    check_count(buffer, 3);

    checked_deinit(&buffer);
    checked_init(&buffer, 3);
    checked_set(buffer, 3, 42);
    checked_read(buffer, 3, 42);
    check_cap(buffer, 6);
    check_count(buffer, 4);

    checked_deinit(&buffer);
    checked_init(&buffer, 3);
    checked_set(buffer, 5, 42);
    checked_read(buffer, 5, 42);
    check_cap(buffer, 6);
    check_count(buffer, 6);

    checked_deinit(&buffer);
    checked_init(&buffer, 3);
    checked_set(buffer, 6, 42);
    checked_read(buffer, 6, 42);
    check_cap(buffer, 12);
    check_count(buffer, 7);

    checked_deinit(&buffer);
    checked_init(&buffer, 3);
    checked_set(buffer, 11, 42);
    checked_read(buffer, 11, 42);
    check_cap(buffer, 12);
    check_count(buffer, 12);

    checked_deinit(&buffer);
    checked_init(&buffer, 3);
    checked_set(buffer, 12, 42);
    checked_read(buffer, 12, 42);
    check_cap(buffer, 24);
    check_count(buffer, 13);

    checked_deinit(&buffer);
    checked_init(&buffer, 3);
    checked_set(buffer, UINT32_MAX - 1, 42);
    checked_read(buffer, UINT32_MAX - 1, 42);
    check_cap(buffer, UINT32_MAX);
    check_count(buffer, UINT32_MAX);

    checked_deinit(&buffer);

    return 0;
}
