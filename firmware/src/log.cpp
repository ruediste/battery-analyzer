#include "log.h"

#include "config.h"

#if IS_FRAMEWORK_NATIVE
#include <cstdarg>
#endif

namespace logging
{
#if IS_FRAMEWORK_NATIVE

    WINDOW *logWindow;
    void setLogWindow(WINDOW *wnd)
    {
        logWindow = wnd;
    }

    void print(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        vw_printw(logWindow, fmt, args);
        wprintw(logWindow, "\n");
        wrefresh(logWindow);
    }
#else
#endif
}