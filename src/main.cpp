/*  Practica 2 de SEP, controla la calefaccion de 2 zonas por medio de estructuras y comandos de consola
    Copyright (C) 2023  Daniel Ernesto Zorraquino Lejeune

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.*/
#include <Arduino.h>
#include <TimerOne.h>
#include "GlobalStructures.h"
#include "ControlFunctions.h"
#include "RelojConEstructuras.h"
#include "BlinkSinDelays.h"
#include "Eeprom.h"

#define pinOnOff 2
#define pinViaje 4
#define pinReset 3

void contarTiempo(void);
void shell(void);

t_heating_system control;

void setup()
{
  //***************************************************************************
  //        INICIAR SERIAL Y INTERRUPCION
  //***************************************************************************
  Serial.begin(115200);
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(contarTiempo);

  //***************************************************************************
  //        PINMODES
  //***************************************************************************
  for (int i = 8; i <= 13; i++)
    pinMode(i, OUTPUT);
  for (int i = 2; i <= 4; i++)
  {
    pinMode(i, INPUT_PULLUP);
  }

  //***************************************************************************
  //        PUESTA EN HORA DEL RELOJ
  //***************************************************************************
  int conv = StringToTiempo(__TIME__, &control.horaReal);
  if (conv != 1)
    Serial.println("ERROR EN LA PUESTA EN HORA");
  //***************************************************************************
  //        PRESET GENERAL
  //***************************************************************************
  if (EEPROM.read(0) == 0)
    EEPROM.get(1, control);
  if (EEPROM.read(0) == 1)
  {
    int address = sizeof(t_heating_system)+1;
    EEPROM.get(address, control);
  }
  control.estadoCalefaccion = Off;
  control.tPrevCambioViaje = millis(); // compensar tiempo de leida de EEPROM
  control.tPrevCambioOnOff = millis(); // compensar tiempo de leida de EEPROM
}
void contarTiempo(void)
{
  control.horaReal.seconds += 1;
}

void loop()
{
  unsigned long tactual = millis();
  //***************************************************************************
  //       LECTURA TIEMPO ACTUAL
  //***************************************************************************
  t_time horaActualCopy;
  noInterrupts();
  guardarTiempo(control.horaReal);
  horaActualCopy = control.horaReal;
  interrupts();
  //***************************************************************************
  //       LLAMADA A SHELL
  //***************************************************************************
  if (Serial.available() > 0)
  {
    shell();
  }
  //***************************************************************************
  //       LECTURA TEMPERATURAS
  //***************************************************************************
  control.pisos[0].temperatura = mapFloat(analogRead(control.pisos[0].sensorT.pin), 0.0, 1023.0, control.pisos[0].sensorT.RangoBajo, control.pisos[0].sensorT.RangoAlto);
  control.pisos[1].temperatura = mapFloat(analogRead(control.pisos[1].sensorT.pin), 0.0, 1023.0, control.pisos[1].sensorT.RangoBajo, control.pisos[1].sensorT.RangoAlto);
  control.temperaturaAcumulador = mapFloat(analogRead(control.sensorAcumulador.pin), 0.0, 1023.0, control.sensorAcumulador.RangoBajo, control.sensorAcumulador.RangoAlto);
  control.colectores[0].temperatura = mapFloat(analogRead(control.colectores[0].sensorT.pin), 0.0, 1023.0, control.colectores[0].sensorT.RangoBajo, control.colectores[0].sensorT.RangoAlto);
  //***************************************************************************
  //       LECTURA Y CONTROL UPS
  //***************************************************************************
  control.alimentacion.lecturaSensor = mapFloat(analogRead(control.alimentacion.pinUPS), 0.0, 1023.0, 0.0, 5.0);
  control.alimentacion.voltajeAlimentacion = mapFloat(control.alimentacion.lecturaSensor, 0.0, 4.0, 0.0, 12.0);
  control.alimentacion.estadoUPS = estadoUPS(control.alimentacion.voltajeAlimentacion, control.alimentacion.margenVoltaje, control.alimentacion.voltajeDeseado);
  //***************************************************************************
  //       ERROR POR SOBRETEMPERATURA Y BLINK
  //***************************************************************************
  control.temperaturaAcumulador = mapFloat(analogRead(control.sensorAcumulador.pin), 0.0, 1023.0, control.sensorAcumulador.RangoBajo, control.sensorAcumulador.RangoAlto);
  if (control.temperaturaAcumulador >= control.temperaturaAcumuladorError)
  {
    blinkSinDelays(control.ledError, tactual, 1000, 1000, &control.tPrevErrorLed, &control.prevLEDerror);
  }
  else if (control.alimentacion.estadoUPS != ALIMENTACION_OK)
  {
    blinkSinDelays(control.ledError, tactual, 1000, 4000, &control.tPrevErrorLed, &control.prevLEDerror);
  }
  else
  {
    digitalWrite(control.ledError, LOW);
  }
  //***************************************************************************
  //       CONTROL DEL COLECTOR
  //***************************************************************************
  switch (control.colectores[0].vacio)
  {
  case false:                                                                          // si no esta vacio
    if (control.colectores[0].temperatura >= control.colectores[0].temperaturaVaciado) // verificamos temperatura
    {
      if (control.colectores[0].valvula != estadosValvula::Abierto) // Abrimos valvula
      {
        activacionElectrovalvula(control.colectores[0].pinValvula, tactual, &control.colectores[0].tPrevValvula, 1000, &control.colectores[0].valvula, &control.colectores[0].valvulaAnterior);
      }
      if (control.colectores[0].bomba == 0) // Activamos bomba para sacar agua caliente y meter agua fria
        control.colectores[0].bomba = 1;
      if (tactual - control.colectores[0].tPrevVaciado >= control.colectores[0].tiempoVaciado)
      {
        control.colectores[0].vacio = true; // si ha pasado el tiempo el colector esta vacio
      }
    }
    else
    {
      control.colectores[0].tPrevVaciado = tactual; // Deja de contar cuando se llega a temperatura
    }
    break;
  case true:                                                      // si esta vacio
    if (control.colectores[0].valvula != estadosValvula::Cerrado) // verificamos valvula para cerrar
    {
      activacionElectrovalvula(control.colectores[0].pinValvula, tactual, &control.colectores[0].tPrevValvula, 1000, &control.colectores[0].valvula, &control.colectores[0].valvulaAnterior);
    }
    if (control.colectores[0].bomba == 1) // apagamos valvula
      control.colectores[0].bomba = 0;
    if (control.colectores[0].valvula == estadosValvula::Cerrado && control.colectores[0].bomba == 0) // si esta todo cerrado y bomba apagada ya esta lleno de agua fria
      control.colectores[0].vacio = false;
    break;
  }

  //***************************************************************************
  //       SISTEMA DE CALEFACCION
  //***************************************************************************
  botonPermutaEstados(digitalRead(pinOnOff), 2000, 1000, &control.tPrevCambioOnOff, &control.estadoCalefaccion, Off, On);
  // Control por hora
  if (compararTiempo(&horaActualCopy, &control.horaOn) >= 0 && control.controlPorHoras)
  {
    control.estadoCalefaccion = On;
  }
  if (compararTiempo(&horaActualCopy, &control.horaOff) >= 0 && control.controlPorHoras)
  {
    control.estadoCalefaccion = Off;
  }
  switch (control.estadoCalefaccion)
  {
  case Off:
    cerradoSistema(&control);
    control.temperaturaAcumulador = mapFloat(analogRead(control.sensorAcumulador.pin), 0.0, 1023.0, control.sensorAcumulador.RangoBajo, control.sensorAcumulador.RangoAlto); // para que no deje de leer la temperatura
    control.estadoAnteriorViaje = control.estadoCalefaccion;
    control.controlPorHorasAnterior = control.controlPorHoras;
    // Boton de viaje
    if (tactual - control.tPrevCambioViaje > 2000)
    {
      if (digitalRead(pinViaje) == HIGH)
      {
        control.estadoCalefaccion = Viaje;
        control.controlPorHoras = false;
        control.tPrevCambioViaje = tactual;
      }
    }
    if (digitalRead(pinViaje) == HIGH)
      control.tPrevCambioViaje = tactual;
    break;

  case On:
    control.temperaturaAcumulador = mapFloat(analogRead(control.sensorAcumulador.pin), 0.0, 1023.0, control.sensorAcumulador.RangoBajo, control.sensorAcumulador.RangoAlto); // para que no deje de leer la temperatura
    if (control.alimentacion.estadoUPS == estadosAlimentacion::ALIMENTACION_OK)
    {
      // Control Zona 1int compararTiempo(t_time t_actual, t_time t_comparar);
      histesis(&control.pisos[0]);
      controlZona(&control.pisos[0], control.pisos[0].necesitaCalefaccion, tactual);
      // Control Zona 2
      histesis(&control.pisos[1]);
      controlZona(&control.pisos[1], control.pisos[1].necesitaCalefaccion, tactual);
      // Encender Apagar calefaccion principal.
      if (control.pisos[0].necesitaCalefaccion || control.pisos[1].necesitaCalefaccion)
      {
        if (control.valvulaPrincipal != estadosValvula::Abierto)
        {
          activacionElectrovalvula(control.pinPrincipal, tactual, &control.tPrevValvula, 1000, &control.valvulaPrincipal, &control.valvulaPrincipalAnterior);
        }
        if (control.bombaPrincipal == 0)
          control.bombaPrincipal = 1;
        if (control.temperaturaAcumulador >= control.temperaturaDisparoCaldera)
          digitalWrite(control.pinCaldera, HIGH);
        else
          digitalWrite(control.pinCaldera, LOW);
      }
      else
      {
        if (control.valvulaPrincipal != estadosValvula::Cerrado)
        {
          activacionElectrovalvula(control.pinPrincipal, tactual, &control.tPrevValvula, 1000, &control.valvulaPrincipal, &control.valvulaPrincipalAnterior);
        }
        if (control.bombaPrincipal == 1)
          control.bombaPrincipal = 0;
      }
    } // ERRORES CIERRE
    else
    {
      cerradoSistema(&control);
    }
    control.estadoAnteriorViaje = control.estadoCalefaccion;
    control.controlPorHorasAnterior = control.controlPorHoras;

    // BOTON VIAJE
    if (tactual - control.tPrevCambioViaje > 2000)
    {
      if (digitalRead(pinViaje) == HIGH)
      {
        control.estadoCalefaccion = Viaje;
        control.controlPorHoras = false;
        control.tPrevCambioViaje = tactual;
      }
    }
    if (digitalRead(pinViaje) == HIGH)
      control.tPrevCambioViaje = tactual;
    break;

  case Viaje:
    control.temperaturaAcumulador = mapFloat(analogRead(control.sensorAcumulador.pin), 0.0, 1023.0, control.sensorAcumulador.RangoBajo, control.sensorAcumulador.RangoAlto); // para que no deje de leer la temperatura
    control.pisos[0].temperaturaObjetivo = control.temperaturaViaje;
    control.pisos[1].temperaturaObjetivo = control.temperaturaViaje;
    if (control.alimentacion.estadoUPS == estadosAlimentacion::ALIMENTACION_OK)
    {
      // Control Zona 1
      histesis(&control.pisos[0]);
      controlZona(&control.pisos[0], control.pisos[0].necesitaCalefaccion, tactual);
      // Control Zona 2
      histesis(&control.pisos[1]);
      controlZona(&control.pisos[1], control.pisos[1].necesitaCalefaccion, tactual);
      // Encender Apagar calefaccion principal.
      if (control.pisos[0].necesitaCalefaccion || control.pisos[1].necesitaCalefaccion)
      {
        if (control.valvulaPrincipal != estadosValvula::Abierto)
        {
          activacionElectrovalvula(control.pinPrincipal, tactual, &control.tPrevValvula, 1000, &control.valvulaPrincipal, &control.valvulaPrincipalAnterior);
        }
        if (control.bombaPrincipal == 0)
          control.bombaPrincipal = 1;
        if (control.temperaturaAcumulador >= control.temperaturaDisparoCaldera)
          digitalWrite(control.pinCaldera, HIGH);
        else
          digitalWrite(control.pinCaldera, LOW);
      }
      else
      {
        if (control.valvulaPrincipal != estadosValvula::Cerrado)
        {
          activacionElectrovalvula(control.pinPrincipal, tactual, &control.tPrevValvula, 1000, &control.valvulaPrincipal, &control.valvulaPrincipalAnterior);
        }
        if (control.bombaPrincipal == 1)
          control.bombaPrincipal = 0;
      }
    }
    else // CIERRE POR ERRORES
    {
      cerradoSistema(&control);
    }
    // VUELTA VIAJE
    if (tactual - control.tPrevCambioViaje > 1000)
    {
      if (digitalRead(pinViaje) == HIGH)
      {
        control.estadoCalefaccion = control.estadoAnteriorViaje;
        control.controlPorHoras = control.controlPorHorasAnterior;
        control.tPrevCambioViaje = tactual;
      }
    }
    if (digitalRead(pinViaje) == HIGH)
      control.tPrevCambioViaje = tactual;
    break;
  }

  //***************************************************************************
  //       IMPRESIONES
  //***************************************************************************
  Imprimir("TZona1", control.pisos[0].temperatura);
  Imprimir("TZona2", control.pisos[1].temperatura);
  Imprimir("TColector", control.colectores[0].temperatura);
  Imprimir("TAcumulador", control.temperaturaAcumulador);
  Imprimir("TensionUPS", control.alimentacion.voltajeAlimentacion);
  Imprimir("Sistema", control.estadoCalefaccion);
  Imprimir("ValvulaZona1", control.pisos[0].valvula);
}

void shell(void)
{
  String cmd = Serial.readString();
  cmd.toUpperCase();
  Serial.println("");
  Serial.println(cmd);
  if (cmd == "HELP")
  {
    Serial.println(F("/////////////////////////////////////////////////////"));
    Serial.print(F("\t"));
    Serial.println(F("LSITA DE COMANDOS"));
    Serial.println(F("/////////////////////////////////////////////////////"));
    Serial.println(F("SET_OBJECTIVE_TEMPERATURE Z X.X -> Define la temperatura objetivo de la zona Z al float X.X"));
    Serial.println(F("SET_TRAVEL_TEMPERATURE X.X -> Define la temperatura minima del modo viaje al float X.X"));
    Serial.println(F("SET_BOILER_TEMPERATURE X.X -> Define la temperatura de disparo de caldera al float X.X"));
    Serial.println(F("SET_COLLECTOR_TEMPERATURE X.X -> Define la temperatura de vaciado del colector al float X.X"));
    Serial.println(F("SET_COLLECTOR_EMPTY_TIME XX.XX -> se cambia el tiempo de vaciado del colector al float X.X"));
    Serial.println(F("SET_ON_HOUR hh:mm:ss -> Define la hora de encendido a la hora proporcionada"));
    Serial.println(F("SET_OFF_HOUR hh:mm:ss -> Define la hora de apagado a la hora proporcionada"));
    Serial.println(F("ENABLE_HOUR_CONTROL -> Activa el control por horas de la calefaccion"));
    Serial.println(F("SET_HISTERESIS Z X.X -> Modifica la histeresis de la zona Z al valor X.X"));
    Serial.println(F("SET_UPPER_RANGE_Z Z X.X -> Modifica el valor maximo del sensor de temperatura de la zona Z al valor X.X"));
    Serial.println(F("SET_LOWER_RANGE_Z Z X.X -> Modifica el valor minimo del sensor de temperatura de la zona Z al valor X.X"));
    Serial.println(F("SET_UPPER_RANGE_A X.X -> Modifica el valor maximo del sensor de temperatura del acumulador al valor X.X"));
    Serial.println(F("SET_UPPER_RANGE_A X.X -> Modifica el valor maximo del sensor de temperatura del acumulador al valor X.X"));
    Serial.println(F("SET_ERROR_TEMPERATURE X.X -> Cambia la temperatura del error del acumulador al valor X.X"));
    Serial.println(F("SET_UPS_ERROR X.X -> Cambia la tension del error de alimentacion al valor X.X"));
    Serial.println(F("SAVE -> Guarda la configuración actual a la EEPROM"));
    Serial.println(F("LOAD -> Carga los datos de la configuracion guardada en la EEPROM aunque se haya pulsado el boton de reset"));
    Serial.println(F("RECOVER -> Carga los datos de la EEPROM y se dejan listos para carga con el inicio de sistema en caso de haber pulsado el RESET"));
    Serial.println(F("STATUS -> Muestra los datos del sistema de control"));
  }
}