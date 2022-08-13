#include "config.h"

#if IS_FRAMEWORK_ARDUINO
#include "batteryChannelHal.h"
#include "utils.h"
#include "batteryChannel.h"
#include <Arduino.h>

void BatteryChannelHal::setOutputPWM(uint16_t value)
{
    switch (channel)
    {
    case 0:
        OCR1A = value;
        break;
    case 1:
        OCR1B = value;
        break;
    }
}
uint16_t BatteryChannelHal::readVoltage()
{
    // select ADC channel with safety mask
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

    // single conversion mode
    ADCSRA |= (1 << ADSC);
    
    // wait until ADC conversion is complete
    while (ADCSRA & (1 << ADSC))
        ;
    
    return ADC<<6;
}

void BatteryChannelHal::loop()
{
}

void BatteryChannelHal::init()
{
    OCR1A = 0; // clear PWM output
    OCR1B = 0; // clear PWM output

    DDRB |= _BV(PB1) | _BV(PB2);                  // Set pins as outputs
    TCCR1A = _BV(COM1A1) | _BV(COM1B1)            // Non-Inv PWM
             | _BV(WGM11);                        // Mode 14: Fast PWM, TOP=ICR1
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10); // Prescaler 1
    ICR1 = 0xffff;                                // TOP counter value (Relieving OCR1A*)

    // adc initialization
    // Select Vref=AVcc
    ADMUX |= (1 << REFS0);

    // set prescaller to 128 and enable ADC (16MHz/128 => 125kHz (should be in range 50kHz to 200 kHz))
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);
}
#endif