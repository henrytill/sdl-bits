#include "test.h"
#include "util.h"

#define CHECKED_INIT(buff, x_cap, y_cap) TEST(util_buffer2d_init(buff, x_cap, y_cap) == 0)

#define CHECKED_SET(buff, x_index, y_index, item)                                                  \
	TEST(util_buffer2d_set(buff, x_index, y_index, item) == 0)

#define CHECKED_READ(buff, x_index, y_index, expected)                                             \
	do {                                                                                       \
		char out;                                                                          \
		TEST(util_buffer2d_read(buff, x_index, y_index, &out) == 0);                       \
		TEST(out == (expected));                                                           \
	} while (0)

#define CHECK_X_CAP(buff, expected) TEST(util_buffer2d_x_cap(buff) == (expected))

#define CHECK_Y_CAP(buff, expected) TEST(util_buffer2d_y_cap(buff) == (expected))

int
main(int argc, char *argv[])
{
	util_buffer2d_t *buffer = NULL;

	(void)argc;
	(void)argv;

	CHECKED_INIT(&buffer, 3, 3);
	CHECK_X_CAP(buffer, 3);
	CHECK_Y_CAP(buffer, 3);

	CHECKED_SET(buffer, 0, 0, 'a');
	CHECKED_SET(buffer, 1, 0, 'b');
	CHECKED_SET(buffer, 2, 0, 'c');
	CHECKED_SET(buffer, 0, 1, 'd');
	CHECKED_SET(buffer, 1, 1, 'e');
	CHECKED_SET(buffer, 2, 1, 'f');
	CHECKED_SET(buffer, 0, 2, 'g');
	CHECKED_SET(buffer, 1, 2, 'h');
	CHECKED_SET(buffer, 2, 2, 'i');

	CHECKED_READ(buffer, 0, 0, 'a');
	CHECKED_READ(buffer, 1, 0, 'b');
	CHECKED_READ(buffer, 2, 0, 'c');
	CHECKED_READ(buffer, 0, 1, 'd');
	CHECKED_READ(buffer, 1, 1, 'e');
	CHECKED_READ(buffer, 2, 1, 'f');
	CHECKED_READ(buffer, 0, 2, 'g');
	CHECKED_READ(buffer, 1, 2, 'h');
	CHECKED_READ(buffer, 2, 2, 'i');

	CHECKED_SET(buffer, 3, 3, 'z');
	CHECK_X_CAP(buffer, 6);
	CHECK_Y_CAP(buffer, 6);

	CHECKED_READ(buffer, 3, 0, 0);
	CHECKED_READ(buffer, 3, 1, 0);
	CHECKED_READ(buffer, 3, 2, 0);
	CHECKED_READ(buffer, 3, 3, 'z');
	CHECKED_READ(buffer, 5, 5, 0);

	return 0;
}