#ifndef blinkImplementacion
#define blinkImplementacion
#include <Arduino.h>
void blinkSinDelays(uint8_t pinLed, unsigned long tiempo, unsigned long T_ON, unsigned long T_OFF, unsigned long *tiempoPrev, bool *estadoLED)
{

    // conmutacion de estados
    if (tiempo - *tiempoPrev >= T_ON)
    {
        if (*estadoLED == HIGH)
        {
            *estadoLED = LOW;
            *tiempoPrev = tiempo;
        }
    }
    if (tiempo - *tiempoPrev >= T_OFF)
    {
        if (*estadoLED == LOW)
        {
            *estadoLED = HIGH;
            *tiempoPrev = tiempo;
        }
    }

    // escribir estado al led
    digitalWrite(pinLed, *estadoLED);
}
#endif