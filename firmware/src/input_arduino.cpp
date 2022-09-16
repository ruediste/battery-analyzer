#define INPUT_CPP
#include "config.h"

#if IS_FRAMEWORK_ARDUINO
#include <Arduino.h>
#include "input.h"
#include "utils.h"

namespace input
{
  static utils::Protected<bool> inputEncoderPressed = false;
  static volatile unsigned long lastEncoderPressedChangedMs;
  bool lastInputEncoderPressed = false;

  void processInputs();

  void init()
  {
    lastEncoderPressedChangedMs = 0;
#ifdef IS_AVR
    /*
      For our application we use the pin change interrupt on PortD
      Input Encoder: 4x with pins PortD2(D2) and PortD3(D3)
      Input Encoder Switch: PortD4(D4)
    */

    // setup pin change interrupt
    PORTD = 0b11100;      // enable soft pull up on PortD2-D4
    PCICR = (1 << PCIE2); // enable pin change interrupt 1 (PCINT 16..28)
    PCMSK2 = 0b11100;     // enable PortD2-D4 for interrupts
#endif

#ifdef IS_STM
    /*
    For our application we use the pin change interrupt on Port B
    Input Encoder: 4x with pins PortB12(B12) and PortB13(B13)
    Input Encoder Switch: PortV14(B14)
  */

    // // setup pin change interrupt
    // AFIO->EXTICR[2] = (AFIO->EXTICR[2] & ~0xFFF) | 0x111; // set pb for EXTI12-EXTI14
    // EXTI->RTSR |= 0b111 < 12;                             // rising trigger mask
    // EXTI->FTSR |= 0b111 < 12;                             // falling trigger mask
    // EXTI->IMR |= 0b111 < 12;                              // interrupt mask
    attachInterrupt(digitalPinToInterrupt(PB5), processInputs, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PB8), processInputs, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PB9), processInputs, CHANGE);
#endif
  }

  void loop()
  {
    // debounce input encoder pressing
    bool newValue = inputEncoderPressed;
    if (newValue != lastInputEncoderPressed)
    {
      unsigned long now = millis();
      unsigned long duration = now - lastEncoderPressedChangedMs;
      if (duration > 100)
      {
        if (newValue)
          inputEncoderClicked = true;
        lastInputEncoderPressed = newValue;
        lastEncoderPressedChangedMs = now;
      }
    }
  }

  /*
   * 00
   * 01
   * 11
   * 10
   * 00
   */
  int8_t getMovement4x(int8_t oldBits, int8_t newBits)
  {
    switch (oldBits << 2 | newBits)
    {
    case 0b0001:
    case 0b0111:
    case 0b1110:
    case 0b1000:
      return 1;
    case 0b0010:
    case 0b1011:
    case 0b1101:
    case 0b0100:
      return -1;
    default:
      return 0;
    }
  }

  int8_t countInputEncoderInternal;
  uint8_t oldBits;

  void processInputs()
  {
    uint8_t newBits = GPIOB->IDR >> 8;
    int8_t tmp = getMovement4x((oldBits)&0b11, (newBits)&0b11);
    if (tmp != 0)
    {
      countInputEncoderInternal += tmp;
      if ((newBits & 0b11) == 0b11) // 11 is the settle state
      {                             // we reached the settle state
        if (countInputEncoderInternal != 0)
          inputEncoderCount.direct() += countInputEncoderInternal > 0 ? 1 : -1;
        countInputEncoderInternal = 0;
      }
    }
    inputEncoderPressed = (GPIOB->IDR & 1 << 5) == 0;

    oldBits = newBits;
  }

#ifdef IS_AVR
  ISR(PCINT2_vect)
  {
    processInputs();
  }
#endif

#ifdef IS_STM

#endif
} // namespace Input

#endif