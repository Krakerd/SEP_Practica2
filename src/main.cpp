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
#include <EEPROM.h>
#include "GlobalStructures.h"
#include "ControlFunctions.h"
#include "RelojConEstructuras.h"
#include "BlinkSinDelays.h"
#include "EeFunctions.h"

#define pinOnOff 2
#define pinViaje 4
#define pinReset 3

void contarTiempo(void);
void shell(void);

t_heating_system control;
String cmd;
float tempZona1RAW;
float tempZona2RAW;
float upsRAW;
float tempAcumuladorRAW;
float tempColectorRAW;
bool botonOnOffRAW;
bool botonViajeRAW;
bool botonResetRAW;

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
  if (conv != 0)
    Serial.println("ERROR EN LA PUESTA EN HORA");
  //***************************************************************************
  //        PRESET GENERAL
  //***************************************************************************
  // PresetFabrica();
  De_eeprom_a_structura_fabrica(&control);
  control.estadoCalefaccion = Off;
  //***************************************************************************
  //        PRESET VALVULAS
  //***************************************************************************
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
    cmd = Serial.readString();
    control.estadoCalefaccion = control.estadoAnteriorViaje;
    cmd.toUpperCase();
    shell();
  }
  //***************************************************************************
  //       LECTURA Y CONTROL UPS
  //***************************************************************************
  control.alimentacion.lecturaSensor = mapFloat(analogRead(control.alimentacion.pinUPS), 0.0, 1023.0, 0.0, 5.0);
  control.alimentacion.voltajeAlimentacion = mapFloat(control.alimentacion.lecturaSensor, 0.0, 4.0, 0.0, 12.0);
  control.alimentacion.estadoUPS = estadoUPS(control.alimentacion.voltajeAlimentacion, control.alimentacion.margenVoltaje, control.alimentacion.voltajeDeseado);
  //***************************************************************************
  //       ERROR POR SOBRETEMPERATURA Y BLINK
  //***************************************************************************
  float acumuladorRaw = analogRead(control.sensorAcumulador.pin);
  control.temperaturaAcumulador = mapFloat(acumuladorRaw, 0.0, 1023.0, control.sensorAcumulador.RangoBajo, control.sensorAcumulador.RangoAlto);
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
  control.colectores[0].temperatura = mapFloat(analogRead(control.colectores[0].sensorT.pin), 0.0, 1023.0, control.colectores[0].sensorT.RangoBajo, control.colectores[0].sensorT.RangoAlto);
  switch (control.colectores[0].estadoColector)
  {
  case 0:
    if (control.colectores[0].temperatura >= control.colectores[0].temperaturaVaciado)
      control.colectores[0].estadoColector = 1;
    break;
  case 1:
    if (control.colectores[0].valvula != estadosValvula::Abierto)
      activacionElectrovalvula(control.colectores[0].pinValvula, tactual, &control.colectores[0].tPrevValvula, 1000, &control.colectores[0].valvula, &control.colectores[0].valvulaAnterior);
    if (control.colectores[0].valvula == estadosValvula::Abierto)
      control.colectores[0].estadoColector = 2;
    control.colectores[0].tPrevVaciado = tactual;
    break;

  case 2:
    if (tactual - control.colectores[0].tPrevVaciado >= control.colectores[0].tiempoVaciado)
    {
      if (control.colectores[0].valvula != estadosValvula::Cerrado)
        activacionElectrovalvula(control.colectores[0].pinValvula, tactual, &control.colectores[0].tPrevValvula, 1000, &control.colectores[0].valvula, &control.colectores[0].valvulaAnterior);
    }
    if (control.colectores[0].valvula == estadosValvula::Cerrado || control.colectores[0].temperatura >= control.colectores[0].temperaturaVaciado)
      control.colectores[0].estadoColector = 0;
    break;
  }
  //***************************************************************************
  //       SISTEMA DE CALEFACCION
  //***************************************************************************
  tempZona1RAW = analogRead(control.pisos[0].sensorT.pin);
  tempZona2RAW = analogRead(control.pisos[1].sensorT.pin);
  control.pisos[0].temperatura = mapFloat(tempZona1RAW, 0.0, 1023.0, control.pisos[0].sensorT.RangoBajo, control.pisos[0].sensorT.RangoAlto);
  control.pisos[1].temperatura = mapFloat(tempZona2RAW, 0.0, 1023.0, control.pisos[1].sensorT.RangoBajo, control.pisos[1].sensorT.RangoAlto);
  botonOnOffRAW = digitalRead(pinOnOff);
  botonViajeRAW = digitalRead(pinViaje);
  botonOnOff(botonOnOffRAW, tactual, 1000, 2000, &control.tPrevCambioOnOff, &control.estadoCalefaccion);
  botonViaje(botonViajeRAW, tactual, 2000, 1000, &control.tPrevCambioViaje, &control.estadoCalefaccion, &control.estadoAnteriorViaje);

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
    /*if (tactual - control.tPrevCambioViaje > 2000)
    {
      if (botonViajeRAW == HIGH)
      {
        control.estadoCalefaccion = Viaje;
        control.controlPorHoras = false;
        control.tPrevCambioViaje = tactual;
      }
    }
    if (botonViajeRAW == HIGH)
      control.tPrevCambioViaje = tactual;*/

    break;

  case On:
    control.temperaturaAcumulador = mapFloat(analogRead(control.sensorAcumulador.pin), 0.0, 1023.0, control.sensorAcumulador.RangoBajo, control.sensorAcumulador.RangoAlto); // para que no deje de leer la temperatura
    control.estadoAnteriorViaje = control.estadoCalefaccion;
    control.controlPorHorasAnterior = control.controlPorHoras;
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

    // BOTON VIAJE
    /*if (tactual - control.tPrevCambioViaje > 2000)
    {
      if (botonViajeRAW == HIGH)
      {
        control.estadoCalefaccion = Viaje;
        control.controlPorHoras = false;
        control.tPrevCambioViaje = tactual;
      }
    }
    if (botonViajeRAW == HIGH)
      control.tPrevCambioViaje = tactual;*/

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
    /*if (tactual - control.tPrevCambioViaje > 1000)
    {
      if (botonViajeRAW == HIGH)
      {
        control.estadoCalefaccion = control.estadoAnteriorViaje;
        control.controlPorHoras = control.controlPorHorasAnterior;
        control.tPrevCambioViaje = tactual;
      }
    }
    if (botonViajeRAW == HIGH)
      control.tPrevCambioViaje = tactual;*/

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
  Imprimir("ValvulaZona2", control.pisos[1].valvula);
  Imprimir("ValvulaColector", control.colectores[0].valvula);
  Imprimir("ValvulaPrincipal", control.valvulaPrincipal);
}

void shell(void)
{
  control.tPrevCambioViaje = millis();
  control.tPrevCambioOnOff = millis();
  Serial.println("");
  Serial.println(cmd);
  if (cmd == "HELP")
  {
    Serial.println(F("/////////////////////////////////////////////////////"));
    Serial.print(F("\t"));
    Serial.println(F("LSITA DE COMANDOS"));
    Serial.println(F("/////////////////////////////////////////////////////"));
    Serial.println(F("SET_OBJECTIVE_TEMPERATURE Z X.X -> Define la temperatura objetivo de la zona Z (0-1) al float X.X"));
    Serial.println(F("SET_TRAVEL_TEMPERATURE X.X -> Define la temperatura minima del modo viaje al float X.X"));
    Serial.println(F("SET_BOILER_TEMPERATURE X.X -> Define la temperatura de disparo de caldera al float X.X"));
    Serial.println(F("SET_COLLECTOR_TEMPERATURE X.X -> Define la temperatura de vaciado del colector al float X.X"));
    Serial.println(F("SET_COLLECTOR_EMPTY_TIME XX -> se cambia el tiempo de vaciado del colector al valor XX (s)"));
    Serial.println(F("SET_ON_HOUR hh:mm:ss -> Define la hora de encendido a la hora proporcionada"));
    Serial.println(F("SET_OFF_HOUR hh:mm:ss -> Define la hora de apagado a la hora proporcionada"));
    Serial.println(F("ENABLE_HOUR_CONTROL -> Activa el control por horas de la calefaccion"));
    Serial.println(F("SET_HISTERESIS Z X.X -> Modifica la histeresis de la zona Z (0-1) al valor X.X"));
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
  else if (cmd.substring(0, cmd.indexOf(' ')) == "SET_OBJECTIVE_TEMPERATURE")
  {
    String zoneS = cmd.substring(cmd.indexOf(' ') + 1, cmd.lastIndexOf(' '));
    int zone = zoneS.toInt();
    unsigned long zoneU = (unsigned long)zone;
    if (zoneU >= sizeof(control.pisos) / sizeof(t_heating_floor))
    {
      Serial.println("ZONA SELECCIONADA FUERA DE INDICE");
    }
    else
    {
      control.pisos[zone].temperaturaObjetivo = getCommandFloat(cmd);
    }
  }
  else if (cmd.substring(0, cmd.indexOf(' ')) == "SET_TRAVEL_TEMPERATURE")
  {
    control.temperaturaViaje = getCommandFloat(cmd);
  }
  else if (cmd.substring(0, cmd.indexOf(' ')) == "SET_BOILER_TEMPERATURE")
  {
    control.temperaturaDisparoCaldera = getCommandFloat(cmd);
  }
  else if (cmd.substring(0, cmd.indexOf(' ')) == "SET_COLLECTOR_TEMPERATURE")
  {
    control.colectores[0].temperaturaVaciado = getCommandFloat(cmd);
  }
  else if (cmd.substring(0, cmd.indexOf(' ')) == "SET_COLLECTOR_EMPTY_TIME")
  {
    unsigned long tiempo = (unsigned long)getCommandFloat(cmd) * 1000;
    control.colectores[0].tiempoVaciado = tiempo;
  }
  else if (cmd.substring(0, cmd.indexOf(' ')) == "STATUS")
  {
    Serial.print(F("Estado del sistema: "));
    Serial.println(control.estadoCalefaccion);
    Serial.print(F("Margen de alimentacion: "));
    Serial.println(control.alimentacion.margenVoltaje);
    Serial.print(F("Voltaje de alimentacion: "));
    Serial.println(control.alimentacion.voltajeAlimentacion);
    Serial.print(F("Voltaje deseado de alimentacion: "));
    Serial.println(control.alimentacion.voltajeDeseado);
    Serial.print(F("Pin del sensor UPS: "));
    Serial.println(control.alimentacion.pinUPS);
    Serial.print(F("Temperatura de Acumulador: "));
    Serial.println(control.temperaturaAcumulador);
    Serial.print(F("Temperatura de error del Acumulador: "));
    Serial.println(control.temperaturaAcumuladorError);
    Serial.print(F("Pin del sensor del acumulador: "));
    Serial.println(control.sensorAcumulador.pin);
    Serial.print(F("Rangos del sensor del acumulador: "));
    Serial.println(control.sensorAcumulador.RangoAlto);
    Serial.println(control.sensorAcumulador.RangoBajo);
    Serial.print(F("Temperatura de disparo de caldera: "));
    Serial.println(control.temperaturaDisparoCaldera);
    Serial.print(F("Temperatura de viaje: "));
    Serial.println(control.temperaturaViaje);
    Serial.print(F("Pin de sensor del colector: "));
    Serial.println(control.colectores[0].sensorT.pin);
    Serial.print(F("Rangos del sensor del colecor: "));
    Serial.println(control.colectores[0].sensorT.RangoAlto);
    Serial.println(control.colectores[0].sensorT.RangoBajo);
    Serial.print(F("Temperatura Vaciado del colector: "));
    Serial.println(control.colectores[0].temperaturaVaciado);
    Serial.print(F("Tiempo de Vaciado del colector: "));
    Serial.println(control.colectores[0].tiempoVaciado);
  }
}