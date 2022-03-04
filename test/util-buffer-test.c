#include <assert.h>

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

#include "util.h"

/* Some helper macros */

#define CHECKED_INIT(buff, size) assert(util_buffer_init(buff, size) == 0)

#define CHECKED_DEINIT(buff) assert(util_buffer_deinit(buff) == 0)

#define CHECKED_PUSH(buff, item) assert(util_buffer_push(buff, item) == 0)

#define CHECKED_READ(buff, index, expected)                                    \
	do {                                                                   \
		char out;                                                      \
		assert(util_buffer_read(buff, index, &out) == 0);              \
		assert(out == (expected));                                     \
	} while (0)

#define CHECKED_SET(buff, index, item)                                         \
	assert(util_buffer_set(buff, index, item) == 0)

int
main(int argc, char *argv[])
{
	util_buffer_t buffer;

	(void)argc;
	(void)argv;

#if defined(_MSC_VER) && defined(_DEBUG)
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

	CHECKED_INIT(&buffer, 3);

	assert(buffer.cap == 3);
	assert(buffer.size == 0);
	assert(*(buffer.data + 0) == 0);
	assert(*(buffer.data + 1) == 0);
	assert(*(buffer.data + 2) == 0);

	CHECKED_READ(&buffer, 0, 0);
	CHECKED_READ(&buffer, 1, 0);
	CHECKED_READ(&buffer, 2, 0);

	CHECKED_PUSH(&buffer, 0);
	CHECKED_PUSH(&buffer, 1);
	CHECKED_PUSH(&buffer, 2);

	assert(buffer.size == 3);

	CHECKED_READ(&buffer, 0, 0);
	CHECKED_READ(&buffer, 1, 1);
	CHECKED_READ(&buffer, 2, 2);

	CHECKED_PUSH(&buffer, 3);

	assert(buffer.cap == 6);
	assert(buffer.size == 4);
	assert(buffer.data != NULL);

	CHECKED_READ(&buffer, 0, 0);
	CHECKED_READ(&buffer, 1, 1);
	CHECKED_READ(&buffer, 2, 2);
	CHECKED_READ(&buffer, 3, 3);

	CHECKED_SET(&buffer, 3, 42);

	CHECKED_READ(&buffer, 0, 0);
	CHECKED_READ(&buffer, 1, 1);
	CHECKED_READ(&buffer, 2, 2);
	CHECKED_READ(&buffer, 3, 42);

	CHECKED_DEINIT(&buffer);

	assert(buffer.cap == 0);
	assert(buffer.size == 0);
	assert(buffer.data == NULL);

	return 0;
}