#ifndef RelojImplementacion
#define RelojImplementacion
#include "GlobalStructures.h"
#include <Arduino.h>
void imprimirTiempo(t_time *hora)
{
    /**
     * @brief Imprime el tiempo con el formato XX:XX:XX
     */
    char buffer[10];
    sprintf(buffer, "%02d:%02d:%02d", hora->hour, hora->minuts, hora->seconds);
    Serial.println(buffer);
}

int compararTiempo(t_time *t_actual, t_time *t_comparar)
{
    /**
     * @brief Compara un objeto de tiempo para ver si es menor, igual o mayor
     *
     * @param t_actual puntero que indica el tiempo que debe ser comparado
     * @param t_comparar indica la marca de tiempo con la que debe ser comparado el primer valor proporcionado
     *
     * @return un valor > 0 si el tiempo actual es mayor que el tiempo a comparar, 0 si es igual y <0 si es
     *         menor el primer valor al segundo
     */
    int resultado = 0;
    if (t_actual->hour > t_comparar->hour)
        resultado = 1;
    else if (t_actual->hour < t_comparar->hour)
        resultado = -1;
    else if (t_actual->minuts > t_comparar->minuts)
        resultado = 1;
    else if (t_actual->minuts < t_comparar->minuts)
        resultado = -1;
    else if (t_actual->seconds > t_comparar->seconds)
        resultado = 1;
    else if (t_actual->seconds < t_comparar->seconds)
        resultado = -1;
    return resultado;
}

int StringToTiempo(String cadena, t_time *resultado)
{
    /**
     * @brief Convierte una cadena de texto a una estructura t_time
     *
     * @param cadena String que contiene la cadena de texto con la hora, formato XX:XX:XX
     * @param resultado puntero que nos otorga la estructura a ser llenada
     *
     * @return 0 si la conversion fue exitosa y 1 si fracaso
     */
    int comparacionCorrecta = 1;
    String horasS = cadena.substring(0, cadena.indexOf(':'));
    int horasL = horasS.toInt();
    String minutosS = cadena.substring(cadena.indexOf(':') + 1, cadena.lastIndexOf(':'));
    int minutosL = minutosS.toInt();
    String segundosS = cadena.substring(cadena.lastIndexOf(':') + 1);
    int segundosL = segundosS.toInt();
    resultado->hour = horasL;
    resultado->minuts = minutosL;
    resultado->seconds = segundosL;
    // Verificar que la conversion ha sido correcta
    char cadena2[10];
    sprintf(cadena2, "%02d:%02d:%02d", resultado->hour, resultado->minuts, resultado->seconds);
    String cadenaComparar = cadena2;
    if (cadenaComparar == cadena)
    {
        comparacionCorrecta = 0;
    }
    return comparacionCorrecta;
}

void guardarTiempo(t_time *hora)
{
    /**
     * @brief Guarda el tiempo y convierte los segundos a minutos, minutos a horas y resetea horas tras
     *      llegar a las 24
     */
    if (hora->seconds == 60)
    {
        hora->minuts = hora->minuts + 1;
        hora->seconds = 0;
    }
    if (hora->minuts == 60)
    {
        hora->hour = hora->hour + 1;
        hora->minuts = 0;
    }
    if (hora->hour == 24)
    {
        hora->hour = 0;
    }
}
#endif