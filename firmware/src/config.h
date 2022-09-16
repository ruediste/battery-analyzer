#ifndef CONFIG_H
#define CONFIG_H

#include "macros.h"

const int displayCols = 20;
const int displayRows = 4;

#if IS_FRAMEWORK_NATIVE
const int channelCount = 1;
#else
const int channelCount = 8;
#endif
#endif