#ifndef CtrlFuncImp
#define CtrlFuncImp
#include <Arduino.h>
#include "GlobalStructures.h"

//**************************************************************************************************************
//           FUNCIONES DE CONTROL
//**************************************************************************************************************
estadosAlimentacion estadoUPS(float voltaje, float margen, float valor)
{
    static estadosAlimentacion resultado = estadosAlimentacion::ALIMENTACION_OK;
    if (voltaje > (valor + margen))
    {
        resultado = estadosAlimentacion::ALIMENTACION_SOBRE;
    }
    if (voltaje < valor)
    {
        resultado = estadosAlimentacion::ALIMENTACION_DESC;
    }
    if (voltaje <= (valor + margen) && voltaje >= valor)
    {
        resultado = estadosAlimentacion::ALIMENTACION_OK;
    }
    return resultado;
}

void botonPermutaEstados(bool estadoBoton, unsigned long tiempoPara1, unsigned long tiempoPara2, unsigned long *tPrev, estadosCalefaccion *estadoSistema, estadosCalefaccion estado1, estadosCalefaccion estado2)
{
    unsigned long tactualBoton = millis();
    if (*estadoSistema == estado1)
    {
        if (tactualBoton - *tPrev > tiempoPara2)
        {
            if (estadoBoton == HIGH)
            {
                *estadoSistema = On;
                *tPrev = tactualBoton;
            }
        }
    }
    else if (*estadoSistema == estado2)
    {
        if (tactualBoton - *tPrev > tiempoPara1)
        {
            if (estadoBoton == HIGH)
            {
                *estadoSistema = Off;
                *tPrev = tactualBoton;
            }
        }
    }
    if (estadoBoton == HIGH)
        *tPrev = tactualBoton;
}

void activacionElectrovalvula(int pin, unsigned long tactual, unsigned long *prev, unsigned long T, estadosValvula *valvula, estadosValvula *estadoPrev){

    switch (*valvula){
    case estadosValvula::Cerrado:
      digitalWrite(pin, HIGH);
      *prev = tactual;
      estadoPrev = valvula;
      *valvula = estadosValvula::Cambiando;
      break;
    case estadosValvula::Cambiando:
      if(tactual-*prev > T){
        digitalWrite(pin, LOW);
        if(*estadoPrev == estadosValvula::Abierto){
          *valvula = estadosValvula::Cerrado;
        } else if(*estadoPrev == estadosValvula::Cerrado){
          *valvula = estadosValvula::Abierto;
        }
      }
      break;
    case estadosValvula::Abierto:
      digitalWrite(pin,HIGH);
      *prev = tactual;
      *estadoPrev = *valvula;
      *valvula = estadosValvula::Cambiando;
      break;
    }
}

bool controlHisteresis(float tObj, float hist, float valor, bool *resultado)
{
    if (valor > tObj + hist)
        *resultado = LOW;
    if (valor < tObj - hist)
        *resultado = HIGH;
    return *resultado;
}

//**************************************************************************************************************
//           FUNCIONES DE USO GENERAL
//**************************************************************************************************************
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void Imprimir(char nombre[], float valor)
{
    // FUNCION HECHA PARA IMPRIMIR CON TELEPLOT (EXTENSION DE VSCODE)
    Serial.print(">");
    Serial.print(nombre);
    Serial.print(":");
    Serial.println(valor);
}
void ImprimirArduino(char nombre[], float valor)
{
    // FUNCION HECHA PARA IMPRIMIR CON SERIAL PLOTTER (ARDUINO)
    Serial.print(nombre);
    Serial.print(":");
    Serial.print(valor);
    Serial.print(",");
}

//**************************************************************************************************************
//
#endif