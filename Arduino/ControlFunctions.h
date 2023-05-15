#ifndef CtrlFunc
#define CtrlFunc
#include "GlobalStructures.h"
estadosAlimentacion estadoUPS(float voltaje, float margen, float valor);
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
void Imprimir( char nombre[] , float valor);
void ImprimirArduino( char nombre[] , float valor);
void botonPermutaEstados(bool estadoBoton, unsigned long tiempoPara1, unsigned long tiempoPara2, unsigned long *tPrev, estadosCalefaccion *estadoSistema, estadosCalefaccion estado1, estadosCalefaccion estado2);
void activacionElectrovalvula(int pin, unsigned long tactual, unsigned long *prev, unsigned long T, estadosValvula *valvula, estadosValvula *estadoPrev);
void cerradoSistema(t_heating_system *sistema);
void controlZona(t_heating_floor *zona, bool activacion, unsigned long time);
void histesis(t_heating_floor *piso);
float getCommandFloat(String command);
#endif