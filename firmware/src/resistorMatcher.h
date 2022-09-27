#ifndef RESISTOR_MATCHER_H
#define RESISTOR_MATCHER_H

#include "macros.h"

#if IS_FRAMEWORK_NATIVE
#include <ncurses.h>
#endif

namespace resistorMatcher
{
#if IS_FRAMEWORK_NATIVE
    void calculate(WINDOW *w);
#endif
}
#endif