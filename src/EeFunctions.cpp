#ifndef eeImp
#define eeImp
#include <Arduino.h>
#include "EeFunctions.h"
#include <EEPROM.h>
void PresetFabrica(void)
{
  t_heating_system fabrica;

  fabrica.horaReal = (t_time){0, 0, 0};

  fabrica.estadoCalefaccion = Off;
  fabrica.alimentacion.margenVoltaje = 1;
  fabrica.alimentacion.voltajeDeseado = 12;
  fabrica.alimentacion.pinUPS = A0;
  fabrica.pinCaldera = 13;
  fabrica.pinPrincipal = 10;
  fabrica.ledError = 12;
  fabrica.prevLEDerror = false;
  fabrica.valvulaPrincipal = Cerrado;

  fabrica.sensorAcumulador.pin = A2;
  fabrica.sensorAcumulador.RangoAlto = 80;
  fabrica.sensorAcumulador.RangoBajo = -5;
  fabrica.temperaturaAcumuladorError = 75;
  fabrica.temperaturaDisparoCaldera = 45;
  fabrica.temperaturaDisparoCalderaViaje = 4;
  fabrica.temperaturaViaje = 10;

  fabrica.colectores[0].pinValvula = 8;
  fabrica.colectores[0].sensorT.pin = A3;
  fabrica.colectores[0].sensorT.RangoAlto = 80;
  fabrica.colectores[0].sensorT.RangoBajo = -5;
  fabrica.colectores[0].temperaturaVaciado = 70;
  fabrica.colectores[0].tiempoVaciado = 10000;
  fabrica.colectores[0].valvula = Cerrado;
  fabrica.colectores[0].estadoColector = 0;

  fabrica.pisos[0].histeresis = 1;
  fabrica.pisos[0].necesitaCalefaccion = false;
  fabrica.pisos[0].pinValvula = 9;
  fabrica.pisos[0].sensorT.pin = A1;
  fabrica.pisos[0].sensorT.RangoAlto = 80;
  fabrica.pisos[0].sensorT.RangoBajo = -5;
  fabrica.pisos[0].temperaturaObjetivo = 20;
  fabrica.pisos[0].valvula = Cerrado;
  fabrica.pisos[0].horaOn = (t_time){21, 30, 0};
  fabrica.pisos[0].horaOff = (t_time){22, 30, 0};

  fabrica.pisos[1].histeresis = 1;
  fabrica.pisos[1].necesitaCalefaccion = false;
  fabrica.pisos[1].pinValvula = 11;
  fabrica.pisos[1].sensorT.pin = A5;
  fabrica.pisos[1].sensorT.RangoAlto = 80;
  fabrica.pisos[1].sensorT.RangoBajo = -5;
  fabrica.pisos[1].temperaturaObjetivo = 20;
  fabrica.pisos[1].valvula = Cerrado;
  fabrica.pisos[1].horaOn = (t_time){21, 30, 0};
  fabrica.pisos[1].horaOff = (t_time){22, 30, 0};

  fabrica.controlPorHoras = false;

  EEPROM.put(1, fabrica);
}
void De_eeprom_a_structura_fabrica(t_heating_system *C)
{
  EEPROM.get(1, *C);
}
void De_eeprom_a_structura(t_heating_system *C)
{
  int addres = sizeof(t_heating_system) + 2;
  EEPROM.get(addres, *C);
}
void GuardarConfig(t_heating_system *sistema)
{
  int addres = sizeof(t_heating_system) + 2;
  EEPROM.put(addres, *sistema);
  EEPROM.update(0, 1);
}
#endif