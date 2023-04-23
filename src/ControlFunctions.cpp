#ifndef CtrlFuncImp
#define CtrlFuncImp
#include <Arduino.h>
#include "GlobalStructures.h"


//**************************************************************************************************************
//           FUNCIONES DE CONTROL
//**************************************************************************************************************
estadosAlimentacion estadoUPS(float voltaje, float margen, float valor){
    static estadosAlimentacion resultado = estadosAlimentacion::ALIMENTACION_OK;
    if(voltaje > (valor+margen)){resultado = estadosAlimentacion::ALIMENTACION_SOBRE;}
    if(voltaje < valor){resultado = estadosAlimentacion::ALIMENTACION_DESC;}
    if(voltaje <= (valor+margen) && voltaje >= valor){resultado = estadosAlimentacion::ALIMENTACION_OK;}
    return resultado;
}

//**************************************************************************************************************
//           FUNCIONES DE USO GENERAL
//**************************************************************************************************************
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void Imprimir( char nombre[] , float valor){
    // FUNCION HECHA PARA IMPRIMIR CON TELEPLOT (EXTENSION DE VSCODE)
    Serial.print(">");
    Serial.print(nombre);
    Serial.print(":");
    Serial.println(valor);
}
void ImprimirArduino( char nombre[] , float valor){
    // FUNCION HECHA PARA IMPRIMIR CON SERIAL PLOTTER (ARDUINO)
    Serial.print(nombre);
    Serial.print(":");
    Serial.print(valor);
    Serial.print(",");
}
#endif