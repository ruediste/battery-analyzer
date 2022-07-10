#include "config.h"

#if IS_FRAMEWORK_NATIVE

#include "display.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
namespace display
{

  char buffer[displayRows][displayCols + 1];
  int cursorX;
  int cursorY;
  bool shown;

  void updateDisplay()
  {
    printf("\x1B[%iA", displayRows + 2); // move up

    printf("\x1B[0G"); // move to first column
    for (int i = 0; i < displayCols; i++)
      printf("-");
    printf("\x1B[1B"); // move 1 lines down

    for (int i = 0; i < displayRows; i++)
    {
      printf("\x1B[0G"); // move to first column
      printf("\x1B[2K"); // clear line
      printf("%s", buffer[i]);

      printf("\x1B[1B"); // move 1 lines down
    }
    printf("\x1B[0G"); // move to first column
    for (int i = 0; i < displayCols; i++)
      printf("-");

    printf("\x1B[1B"); // move 1 lines down
    printf("\x1B[0G"); // move to first column
  }
  void init()
  {
    for (int row = 0; row < displayRows; row++)
    {
      buffer[row][displayCols] = 0;
    }
  }

  size_t print(const char *str)
  {
    for (int i = 0; i < strlen(str); i++)
    {
      buffer[cursorY][cursorX++] = str[i];
      if (cursorX >= displayCols)
      {
        cursorX = 0;
        cursorY++;
      }
      if (cursorY >= displayRows)
      {
        cursorY = 0;
      }
    }
    updateDisplay();
    return strlen(str);
  }

  size_t print(long n, int base)
  {
    int length = snprintf(NULL, 0, "%ld", n);
    char *str = (char*)malloc(length + 1);
    snprintf(str, length + 1, "%ld", n);
    print(str);
    free(str);
    return length;
  }

  void clear()
  {
    for (int row = 0; row < displayRows; row++)
      for (int col = 0; col < displayCols; col++)
      {
        buffer[row][col] = ' ';
      }
    updateDisplay();
  }
  void setCursor(uint8_t x, uint8_t y)
  {
    cursorX = x;
    cursorY = y;
  }

  void show()
  {
    shown = true;
    for (int i=0; i<displayRows+3; i++) printf("\n");
    updateDisplay();
  }
  void hide()
  {
    shown = false;
    printf("\x1B[%iA", displayRows + 2); // move up
    for (int i = 0; i < displayRows + 2; i++)
    {
      printf("\x1B[2K"); // clear line
      printf("\x1B[1B"); // move 1 lines down
    }
    printf("\x1B[0G"); // move to first column
  }
}
#endif