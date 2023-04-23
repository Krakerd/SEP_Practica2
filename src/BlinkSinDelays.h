#ifndef blinkNoDelay
#define blinkNoDelay
#include <Arduino.h>
void blinkSinDelays(uint8_t pinLed, unsigned long tiempo, unsigned long T_ON, unsigned long T_OFF, unsigned long *tiempoPrev, bool *estadoLED);
#endif