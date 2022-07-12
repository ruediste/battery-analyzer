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
    if (!shown)
      return;
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
    return strlen(str);
  }
  
  void loop(){
    updateDisplay();
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
    return displayPrintf("%f",n);
  }

  size_t print(double n)
  {
    return displayPrintf("%f",n);
  }

  void clear()
  {
    for (int row = 0; row < displayRows; row++)
      for (int col = 0; col < displayCols; col++)
      {
        buffer[row][col] = ' ';
      }
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