#ifndef LOG_H
#define LOG_H
#include "config.h"
#if IS_FRAMEWORK_NATIVE
#include <ncurses.h>
#endif
namespace logging
{
#if IS_FRAMEWORK_NATIVE
#define LOG(...) logging::print(__VA_ARGS__)
    void setLogWindow(WINDOW *logWindow);
#else
#define LOG(...)
#endif

    void print(const char *fmt, ...);
}
#endif