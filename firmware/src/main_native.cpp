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
#include <ncurses.h>

pthread_mutex_t lock;

int msleep(unsigned int tms)
{
    return usleep(tms * 1000);
}

void *loopInvoker(void *threadid)
{
    printf("Loop Invoker Started");
    while (true)
    {
        controller::loop();

        pthread_mutex_unlock(&lock);
        msleep(100);
        pthread_mutex_lock(&lock);
    }
    pthread_exit(NULL);
}

int main()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    WINDOW *logWindow = newwin(10, COLS, 5, 0);
    box(logWindow, 0, 0);
    scrollok(logWindow, TRUE);
    wrefresh(logWindow);

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

    bool quit = false;
    while (!quit)
    {
        pthread_mutex_unlock(&lock);
        int ch = getch();
        pthread_mutex_lock(&lock);

        switch (ch)
        {
        case ' ':
        case KEY_ENTER:
        case KEY_RIGHT:
            input::setInputEncoderClicked();
            break;
        case KEY_UP:
            input::moveEncoder(-1);
            break;
        case KEY_DOWN:
            input::moveEncoder(1);
            break;
        case 'q':
            quit = true;
            break;
        case 'b':
            BatteryChannel::channels[0].control.print(logWindow);
            break;
        }
    }
    endwin();
    return 0;
}
#endif