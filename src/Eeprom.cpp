#ifndef eeImp
#define eeImp
#include <Arduino.h>
#include "Eeprom.h"
#include <EEPROM.h>
void PresetFabrica(void){
    t_heating_system fabrica;

    fabrica.alimentacion.margenVoltaje = 1.0;
    fabrica.alimentacion.voltajeDeseado = 12.0;
    fabrica.alimentacion.pinUPS = A0;
    fabrica.pinCaldera = 13;
    fabrica.pinPrincipal = 10;
    fabrica.ledError = 12;
    fabrica.prevLEDerror = false;

    fabrica.sensorAcumulador.pin = A2;
    fabrica.sensorAcumulador.RangoAlto = 80.0;
    fabrica.sensorAcumulador.RangoBajo = -5.0;
    fabrica.temperaturaAcumuladorError = 75.0;
    fabrica.temperaturaDisparoCaldera = 45.0;

    fabrica.colectores[0].pinValvula = 8;
    fabrica.colectores[0].sensorT.pin = A3;
    fabrica.colectores[0].sensorT.RangoAlto = 80.0;
    fabrica.colectores[0].sensorT.RangoBajo = -5.0;
    fabrica.colectores[0].temperaturaVaciado = 70.0;
    fabrica.colectores[0].tiempoVaciado = 10000;

    fabrica.pisos[0].histeresis=1.0;
    fabrica.pisos[0].necesitaCalefaccion = false;
    fabrica.pisos[0].pinValvula = 9;
    fabrica.pisos[0].sensorT.pin = A1;
    fabrica.pisos[0].sensorT.RangoAlto = 80.0;
    fabrica.pisos[0].sensorT.RangoBajo = -5.0;
    fabrica.pisos[0].temperaturaObjetivo = 20.0;
    fabrica.pisos[0].valvula = Cerrado;

    fabrica.pisos[1].histeresis=1.0;
    fabrica.pisos[1].necesitaCalefaccion = false;
    fabrica.pisos[1].pinValvula = 11;
    fabrica.pisos[1].sensorT.pin = A5;
    fabrica.pisos[1].sensorT.RangoAlto = 80.0;
    fabrica.pisos[1].sensorT.RangoBajo = -5.0;
    fabrica.pisos[1].temperaturaObjetivo = 20.0;
    fabrica.pisos[1].valvula = Cerrado;

    fabrica.controlPorHoras = false;
    fabrica.horaOn = (t_time){21, 30, 0};
    fabrica.horaOn = (t_time){22, 30, 0};


    EEPROM.put(1, fabrica);
}

void GuardarConfig(t_heating_system *sistema)
{
    int addres = sizeof(t_heating_system)+1;
    EEPROM.put(addres, *sistema);
    EEPROM.update(1,1);
}
#endif