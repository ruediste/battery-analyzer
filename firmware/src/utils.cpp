#include "utils.h"
#if IS_FRAMEWORK_NATIVE
#include <sys/time.h>
#endif

namespace utils
{
#if IS_FRAMEWORK_NATIVE
    
    uint64_t nowInternal()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000; // convert tv_sec & tv_usec to millisecond
    }
    uint64_t baseTimeMs=nowInternal();
    instantMs_t now()
    {
        return nowInternal()-baseTimeMs;
    }
#endif

}