#ifndef INPUT_H
#define INPUT_H

#include "types.h"
#include "utils.h"
#include "config.h"

#ifdef INPUT_CPP
#define EXTERN(var, init) var = init
#else
#define EXTERN(var, init) extern var
#endif

namespace input
{

    EXTERN(utils::Protected<int8_t> inputEncoderCount, 0);
    EXTERN(bool inputEncoderClicked, false);

    inline int8_t getAndResetInputEncoder()
    {
        utils::disableInterrupts();
        int8_t tmp = inputEncoderCount.direct();
        inputEncoderCount.direct() = 0;
        utils::enableInterrupts();
        return tmp;
    }

    inline void resetInputEncoderClicked()
    {
        inputEncoderClicked = false;
    }

    inline bool getAndResetInputEncoderClicked()
    {
        if (inputEncoderClicked)
        {
            inputEncoderClicked = false;
            return true;
        }
        return false;
    }

    void init();

    // to be called from the loop
    void loop();

    // to be called from other interrupts to trigger input handling logic
    void checkInputs();

#if IS_FRAMEWORK_NATIVE
    inline void moveEncoder(int delta)
    {
        inputEncoderCount = inputEncoderCount + delta;
    }
    inline void setInputEncoderClicked()
    {
        inputEncoderClicked = true;
    }
#endif

}; // namespace Input
#endif