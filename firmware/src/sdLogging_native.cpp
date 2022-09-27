#include "config.h"

#if IS_FRAMEWORK_NATIVE
#include "utils.h"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

namespace sdLogging
{
    FILE *fd;
    int getFailure()
    {
        return 0;
    }

    int getError()
    {
        return 0;
    }

    void init()
    {
        fd = fopen("log.txt", "a+");
        if (fd == NULL)
        {
            perror("Failed to create/open a file...\n");
        }
        else
        {
            fprintf(fd, "\ntime\n");
        }
    }

    instantMs_t nextPrint = 0;

    void loop()
    {
        if (fd == NULL)
        {
            return;
        }
        instantMs_t now = utils::now();
        if (now > nextPrint)
        {
            nextPrint = now + 1000;

            fprintf(fd, "%i\n", now);
        }
    }
}
#endif