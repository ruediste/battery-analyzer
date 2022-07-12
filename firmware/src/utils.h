#ifndef UTILS_H
#define UTILS_H

#include "config.h"
#ifdef ARDUINO
#include "Arduino.h"
#else
#include <iostream>
#endif

#include "types.h"

namespace utils
{

#if IS_FRAMEWORK_ARDUINO
    inline instantMs_t now()
    {
        return millis();
    }
    inline void disableInterrupts() { noInterrupts(); }
    inline void enableInterrupts() { interrupts(); }
#endif
#if IS_FRAMEWORK_NATIVE
    instantMs_t now();
    inline void disableInterrupts() {}
    inline void enableInterrupts() {}
#endif

    struct DisableInterrupts
    {
        DisableInterrupts()
        {
            disableInterrupts();
        }
        ~DisableInterrupts()
        {
            enableInterrupts();
        }
    };

    template <class T>
    class Protect
    {
        T &value;

    public:
        Protect(T &v) : value(v)
        {
        }

        Protect &operator=(T other)
        {
            if (sizeof(T) > 1)
            {
                DisableInterrupts disableInterrupts;
                this->value = other;
            }
            else
                this->value = other;
            return *this;
        }

        operator T()
        {
            if (sizeof(T) > 1)
            {
                DisableInterrupts disableInterrupts;
                return value;
            }
            else
                return value;
        }
    };

    template <class T>
    Protect<T> protect(T &value) { return Protect<T>(value); }

    template <class T>
    class Protected
    {
        volatile T value;

    public:
        Protected() : value(T()) {}
        Protected(T v) : value(v) {}

        Protected &operator=(T other)
        {
            if (sizeof(T) > 1)
            {
                DisableInterrupts disableInterrupts;
                this->value = other;
            }
            else
                this->value = other;
            return *this;
        }

        operator T()
        {
            if (sizeof(T) > 1)
            {
                DisableInterrupts disableInterrupts;
                return (const T &)value;
            }
            else
                return (const T &)value;
        }

        T &direct()
        {
            return (T &)value;
        }
    };

}

#undef abs
#undef max
#undef min

template <class T>
T abs(T value)
{
    return value >= 0 ? value : -value;
}
template <class T>
T max(T a, T b)
{
    return a >= b ? a : b;
}
template <class T>
T min(T a, T b)
{
    return a <= b ? a : b;
}

#ifndef ARDUINO
#define LOG(x) std::cout << #x << ": " << (x) << "\n";
#else
#define LOG(x)
#endif

#ifndef ARDUINO
#define sLOG(x) ;
#else
#include <avr/pgmspace.h>
#define sLOG(x)              \
    Serial.print(F(#x " ")); \
    Serial.println(x);
#endif

#endif