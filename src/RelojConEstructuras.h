#ifndef RelojPrototipos
#define RelojPrototipos
#include "GlobalStructures.h"
void imprimirTiempo(t_time hora);
int compararTiempo(t_time t_actual, t_time t_comparar);
t_time StringToTiempo(String cadena);
void guardarTiempo(t_time hora);
#endif