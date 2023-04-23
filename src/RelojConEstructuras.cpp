#ifndef RelojImplementacion
#define RelojImplementacion
#include "GlobalStructures.h"
#include <Arduino.h>
void imprimirTiempo(t_time hora)
{
    Serial.print(hora.hour);
    Serial.print(":");
    Serial.print(hora.minuts);
    Serial.print(":");
    Serial.println(hora.seconds);
}

int compararTiempo(t_time t_actual, t_time t_comparar)
{
    int resultado = 0;
    long int segundosActual = t_actual.hour * 3600 + t_actual.minuts * 60 + t_actual.seconds;
    long int segundosComparar = t_comparar.hour * 3600 + t_comparar.minuts * 60 + t_comparar.seconds;
    resultado = segundosActual - segundosComparar;

    return resultado;
}

t_time StringToTiempo(String cadena)
{
    String horasS = cadena.substring(0, cadena.indexOf(':'));
    int horasL = horasS.toInt();
    String minutosS = cadena.substring(cadena.indexOf(':') + 1, cadena.lastIndexOf(':'));
    int minutosL = minutosS.toInt();
    String segundosS = cadena.substring(cadena.lastIndexOf(':') + 1);
    int segundosL = segundosS.toInt();
    t_time resultado = {.hour = horasL, .minuts = minutosL, .seconds = segundosL};
    return resultado;
}

void guardarTiempo(t_time hora)
{
    if (hora.seconds == 60)
    {
        hora.minuts = hora.minuts + 1;
        hora.seconds = 0;
    }
    if (hora.minuts == 60)
    {
        hora.hour = hora.hour + 1;
        hora.minuts = 0;
    }
    if (hora.hour == 24)
    {
        hora.hour = 0;
    }
}
#endif