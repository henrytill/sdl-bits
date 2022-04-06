#include <stddef.h>
#include <stdlib.h>

#include "test.h"
#include "buffer.h"

#define checked_init(buff, x_cap, y_cap) test(buffer2d_init(buff, x_cap, y_cap) == 0)

#define checked_deinit(buff) test(buffer2d_deinit(buff) == 0)

#define checked_set(buff, x_index, y_index, item)                                                  \
    test(buffer2d_set(buff, x_index, y_index, item) == 0)

#define checked_read(buff, x_index, y_index, expected)                                             \
    do {                                                                                           \
        char out;                                                                                  \
        test(buffer2d_read(buff, x_index, y_index, &out) == 0);                                    \
        test(out == (expected));                                                                   \
    } while (0)

#define check_x_cap(buff, expected) test(buffer2d_x_cap(buff) == (expected))

#define check_y_cap(buff, expected) test(buffer2d_y_cap(buff) == (expected))

int main(int argc, char *argv[]) {
    struct Buffer2d *buffer = NULL;

    (void)argc;
    (void)argv;

    checked_init(&buffer, 3, 3);
    check_x_cap(buffer, 3);
    check_y_cap(buffer, 3);

    checked_set(buffer, 0, 0, 'a');
    checked_set(buffer, 1, 0, 'b');
    checked_set(buffer, 2, 0, 'c');
    checked_set(buffer, 0, 1, 'd');
    checked_set(buffer, 1, 1, 'e');
    checked_set(buffer, 2, 1, 'f');
    checked_set(buffer, 0, 2, 'g');
    checked_set(buffer, 1, 2, 'h');
    checked_set(buffer, 2, 2, 'i');

    checked_read(buffer, 0, 0, 'a');
    checked_read(buffer, 1, 0, 'b');
    checked_read(buffer, 2, 0, 'c');
    checked_read(buffer, 0, 1, 'd');
    checked_read(buffer, 1, 1, 'e');
    checked_read(buffer, 2, 1, 'f');
    checked_read(buffer, 0, 2, 'g');
    checked_read(buffer, 1, 2, 'h');
    checked_read(buffer, 2, 2, 'i');

    checked_set(buffer, 3, 3, 'z');
    check_x_cap(buffer, 6);
    check_y_cap(buffer, 6);

    checked_read(buffer, 3, 0, 0);
    checked_read(buffer, 3, 1, 0);
    checked_read(buffer, 3, 2, 0);
    checked_read(buffer, 3, 3, 'z');
    checked_read(buffer, 5, 5, 0);

    checked_deinit(&buffer);
    checked_init(&buffer, 3, 3);
    checked_set(buffer, 6, 6, 'z');
    check_x_cap(buffer, 12);
    check_y_cap(buffer, 12);

    checked_read(buffer, 6, 0, 0);
    checked_read(buffer, 6, 1, 0);
    checked_read(buffer, 6, 2, 0);
    checked_read(buffer, 6, 6, 'z');
    checked_read(buffer, 11, 11, 0);

    checked_deinit(&buffer);

    return EXIT_SUCCESS;
}
