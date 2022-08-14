#include "config.h"

#if IS_FRAMEWORK_NATIVE

#include "display.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

namespace display
{
  WINDOW *lcdWindow;

  const int rows=4;
  const int cols=20;
  void init()
  {
    
    {
        WINDOW *wnd = newwin(rows+2, cols+2, 0, 0);
        box(wnd, 0, 0);
        wrefresh(wnd);
        delwin(wnd);
    }

    lcdWindow = newwin(rows, cols, 1, 1);
    wrefresh(lcdWindow);
  }

  size_t print(const char *str)
  {
    wprintw(lcdWindow,str);
    wrefresh(lcdWindow);
    return strlen(str);
  }
  

  template <class T> size_t displayPrintf(const char* format, T value)
  {
    int length = snprintf(NULL, 0, format, value);
    char *str = (char*)malloc(length + 1);
    snprintf(str, length + 1, format, value);
    print(str);
    free(str);
    return length;
  }

  size_t print(long n)
  {
    return displayPrintf("%ld",n);
  }
  
  size_t print(int n)
  {
    return displayPrintf("%d",n);
  }

  size_t print(float n)
  {
    return displayPrintf("%.2f",n);
  }

  size_t print(double n)
  {
    return displayPrintf("%.2f",n);
  }

  void print(const char ch) {
    displayPrintf("%.c",ch);
  }

  void clear()
  {
    werase(lcdWindow);
  }

  void setCursor(uint8_t x, uint8_t y)
  {
    wmove(lcdWindow,y,x);
  }
}
#endif