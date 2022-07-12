#include "macros.h"
#if IS_FRAMEWORK_NATIVE

#include <stdio.h>
#include <termio.h>
#include <unistd.h> //STDIN_FILENO
#include <stdlib.h>
#include <string.h>
#include "display.h"
#include "controller.h"
#include "input.h"
#include <pthread.h>
#include "batteryChannel.h"

pthread_mutex_t lock;

int msleep(unsigned int tms)
{
    return usleep(tms * 1000);
}

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
        controller::loop();
        display::loop();
        pthread_mutex_unlock(&lock);
        int ch = getchar_immediate();
        pthread_mutex_lock(&lock);
        // printf("%x\n", ch);
        if (
             ch == 4   // ctrl+d
            || ch == 'd' // lower case d
        )
            break;
        if (ch == 3)
        {
            // ctrl+c
            exit(1);
        }
        if (ch == 0x20// space
         || ch == '\r'   // newline
         )
        {
            input::setInputEncoderClicked();
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
                input::moveEncoder(-1);
            }
            if (ch == 0x42)
            {
                // TODO: down
                input::moveEncoder(1);
            }
            if (ch == 0x43)
            {
                // TODO: right
                input::setInputEncoderClicked();
            }
            if (ch == 0x44)
            {
                // TODO: left
            }
        }
    }
    display::hide();
}

void *loopInvoker(void *threadid)
{
    printf("Loop Invoker Started");
    while (true)
    {
        controller::loop();
        display::loop();

        pthread_mutex_unlock(&lock);
        msleep(100);
        pthread_mutex_lock(&lock);
    }
    pthread_exit(NULL);
}

int main()
{
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }
     pthread_mutex_lock(&lock);

    controller::init();

    {
        pthread_t tid;
        pthread_create(&tid, NULL, &loopInvoker, NULL);
    }

    showDisplay();
    while (true)
    {
        char *line = NULL;
        size_t len = 0;
        printf("> ");
        pthread_mutex_unlock(&lock);
        ssize_t lineSize = getline(&line, &len, stdin);
        pthread_mutex_lock(&lock);
        line[lineSize - 1] = 0;
        if (strcmp("q", line) == 0)
        {
            break;
        }
        else if (strcmp("d", line) == 0)
        {
            showDisplay();
        }
        else if (strcmp("p", line) == 0)
        {
            BatteryChannel::channels[0].print();
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