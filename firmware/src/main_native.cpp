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
#include "resistorMatcher.h"

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
        msleep(10);
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
    curs_set(0);
    refresh();

    {
        WINDOW *wnd = newwin(LINES - 6, COLS, 6, 0);
        box(wnd, 0, 0);
        wrefresh(wnd);
        delwin(wnd);
    }
    WINDOW *logWindow = newwin(LINES - 8, COLS - 2, 7, 1);
    scrollok(logWindow, TRUE);
    wrefresh(logWindow);

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\nmutex init has failed\n");
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
        case '\n':
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
        case 'r':
            resistorMatcher::calculate(logWindow);
            break;
        }
    }
    endwin();
    return 0;
}
#endif