#include <assert.h>
#include <limits.h>

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

#include "util.h"

int
main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

#if defined(_MSC_VER) && defined(_DEBUG)
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

	assert(util_uint32_sat_sub(1, UINT_MAX) == 0);
	assert(util_uint32_sat_sub(0, UINT_MAX) == 0);
	assert(util_uint32_sat_sub(UINT_MAX, UINT_MAX) == 0);
	assert(util_uint32_sat_sub(UINT_MAX, UINT_MAX - 1) == 1);
	assert(util_uint32_sat_sub(UINT_MAX, 1) == UINT_MAX - 1);

	return 0;
}