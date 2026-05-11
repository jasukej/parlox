// refresher that this first checks that preprocessor macro is undefined
#ifndef parlox_common_h
#define parlox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef NDEBUG_TRACE
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION
#endif

#define UINT8_COUNT (UINT8_MAX + 1)

#endif
