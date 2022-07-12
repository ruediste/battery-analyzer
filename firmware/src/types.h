#ifndef TYPES_H
#define TYPES_H

#include "config.h"

#include <inttypes.h>
#include <stddef.h>

/*
Timestamp in milli seconds.
*/
typedef int32_t instantMs_t;

#if IS_FRAMEWORK_NATIVE
#define F(X) X
#else
#endif

#endif