#ifndef CtrlFunc
#define CtrlFunc
#include "GlobalStructures.h"
estadosAlimentacion estadoUPS(float voltaje, float margen, float valor);
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
void Imprimir( char nombre[] , float valor);
void ImprimirArduino( char nombre[] , float valor);
#endif