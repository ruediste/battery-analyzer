#include "macros.h"
#if IS_FRAMEWORK_NATIVE

#include <stdio.h>
#include <termio.h>
#include <unistd.h> //STDIN_FILENO
#include <stdlib.h>
#include <string.h>
#include "display.h"

int getchar_immediate()
{
    static struct termios oldTermios, newTermios;

    tcgetattr(STDIN_FILENO, &oldTermios);
    newTermios = oldTermios;

    cfmakeraw(&newTermios);

    tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
    int c = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
    return c;
}

void showDisplay()
{
    display::show();
    while (true)
    {
        int ch = getchar_immediate();
        // printf("%x\n", ch);
        if (ch == '\r'   // newline
            || ch == 4   // ctrl+d
            || ch == 'd' // lower case d
        )
            break;
        if (ch == 3)
        {
            // ctrl+c
            exit(1);
        }
        if (ch == 0x20)
        {
            // TODO: space
            display::print("Foo");
        }
        if (ch == 0x1b)
        {
            ch = getchar_immediate();
            if (ch != 0x5b)
                continue;
            ch = getchar_immediate();
            if (ch == 0x41)
            {
                // TODO: up
            }
            if (ch == 0x42)
            {
                // TODO: down
            }
            if (ch == 0x43)
            {
                // TODO: right
            }
            if (ch == 0x44)
            {
                // TODO: left
            }
        }
    }
    display::hide();
}
int main()
{
    printf("Hello World from PlatformIO!\n");
    display::init();
    while (true)
    {
        char *line = NULL;
        size_t len = 0;
        printf("> ");
        ssize_t lineSize = getline(&line, &len, stdin);
        line[lineSize - 1] = 0;
        if (strcmp("q", line) == 0)
        {
            break;
        }
        else if (strcmp("d", line) == 0)
        {
            showDisplay();
        }
        else
        {
            printf("Unknown command: %s\n", line);
        }
        free(line);
    }
    return 0;
}
#endif