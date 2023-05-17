#ifndef CtrlFunc
#define CtrlFunc
#include "GlobalStructures.h"
estadosAlimentacion estadoUPS(float voltaje, float margen, float valor);
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
void Imprimir( const char nombre[] , float valor);
void ImprimirArduino( char nombre[] , float valor);
void botonOnOff(bool estadoBoton,unsigned long tactualBoton, unsigned long tiempoParaOn, unsigned long tiempoParaOff, unsigned long *tPrev, estadosCalefaccion *estadoSistema);
void botonViaje(bool estadoBoton, unsigned long tactualBoton, unsigned long tEntradaViaje, unsigned long tSalidaViaje, unsigned long *tPrev, estadosCalefaccion *sistema, estadosCalefaccion *sistemaPrevViaje);
void botonReset(bool estadoBoton, unsigned long tactual, unsigned long T, t_heating_system *sistema);
void activacionElectrovalvula(int pin, unsigned long tactual, unsigned long *prev, unsigned long T, estadosValvula *valvula, estadosValvula *estadoPrev);
void cerradoSistema(t_heating_system *sistema);
void controlZona(t_heating_floor *zona, bool activacion, unsigned long time);
void histesis(t_heating_floor *piso);
float getCommandFloat(String command);
#endif