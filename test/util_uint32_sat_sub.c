#include <limits.h>

#include "test.h"
#include "util.h"

int
main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	TEST(util_uint32_sat_sub(0, 0) == 0);
	TEST(util_uint32_sat_sub(0, 1) == 0);
	TEST(util_uint32_sat_sub(1, 1) == 0);
	TEST(util_uint32_sat_sub(1, 2) == 0);
	TEST(util_uint32_sat_sub(2, 2) == 0);
	TEST(util_uint32_sat_sub(2, 1) == 1);
	TEST(util_uint32_sat_sub(2, 0) == 2);

	TEST(util_uint32_sat_sub(1, UINT_MAX) == 0);
	TEST(util_uint32_sat_sub(0, UINT_MAX) == 0);
	TEST(util_uint32_sat_sub(UINT_MAX, UINT_MAX) == 0);
	TEST(util_uint32_sat_sub(UINT_MAX, UINT_MAX - 1) == 1);
	TEST(util_uint32_sat_sub(UINT_MAX, 1) == UINT_MAX - 1);

	return 0;
}
