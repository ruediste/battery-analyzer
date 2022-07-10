#ifndef TYPES_H
#define TYPES_H

#include "config.h"

#include <inttypes.h>
#include <stddef.h>

/*
Timestamp in micro seconds. Wraps every 2e9*1e-6=4e3 seconds = 33 minutes
*/
typedef int32_t instantUs_t;

#if IS_FRAMEWORK_NATIVE
#define F(X) X
#else
#endif

#endif