#include <Arduino.h>
#include <TimerOne.h>
#include "GlobalStructures.h"
#include "ControlFunctions.h"
#include "RelojConEstructuras.h"
#include "BlinkSinDelays.h"

#define pinOnOff 2
#define pinViaje 3
#define pinReset 4

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
    pinMode(i, INPUT_PULLUP);

  //***************************************************************************
  //        PUESTA EN HORA DEL RELOJ
  //***************************************************************************
  int conv = StringToTiempo(__TIME__, &control.horaReal);
  if (conv != 1)
    Serial.println("ERROR EN LA PUESTA EN HORA");

  //***************************************************************************
  //        PRESET GENERAL
  //***************************************************************************
  control.pinCaldera = 13;
  control.pinPrincipal = 10;
  control.alimentacion.pinUPS = A0;
  control.alimentacion.voltajeDeseado = 12.0;
  control.alimentacion.margenVoltaje = 1.0;
  control.ledError = 12;
  control.sensorAcumulador.pin = A2;
  control.sensorAcumulador.RangoAlto = 80.0; // eeprom
  control.sensorAcumulador.RangoBajo = -5.0; // eeprom
  control.colectores[0].sensorT.pin = A3;
  control.colectores[0].sensorT.RangoBajo = -5.0; // eeprom
  control.colectores[0].sensorT.RangoAlto = 80.0; // eeprom
  control.estadoCalefaccion = estadosCalefaccion::Off;

  //***************************************************************************
  //        PRESET ZONAS
  //***************************************************************************
  control.pisos[0].sensorT.pin = A1;
  control.pisos[0].pinValvula = 9;
  control.pisos[0].valvula = Cerrado;
  control.pisos[0].temperaturaObjetivo = 20.0; // temperatura objetivo debe ir a eeprom
  control.pisos[0].histeresis = 1.0;           // eeprom
  control.pisos[0].sensorT.RangoAlto = 80.0;   // eeprom
  control.pisos[0].sensorT.RangoBajo = -5.0;   // eeprom

  control.pisos[1].sensorT.pin = A5;
  control.pisos[1].pinValvula = 11;
  control.pisos[1].valvula = Cerrado;
  control.pisos[1].temperaturaObjetivo = 20.0; // temperatura objetivo debe ir a eeprom
  control.pisos[1].histeresis = 1.0;           // eeprom
  control.pisos[1].sensorT.RangoAlto = 80.0;   // eeprom
  control.pisos[1].sensorT.RangoBajo = -5.0;   // eeprom
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
  if (control.alimentacion.estadoUPS != estadosAlimentacion::ALIMENTACION_OK)
  {
    blinkSinDelays(control.ledError, tactual, 1000, 4000, &control.tPrevErrorLed, &control.prevLEDerror);
  }
  else if (control.temperaturaAcumulador >= control.temperaturaAcumuladorError)
  {
    blinkSinDelays(control.ledError, tactual, 1000, 4000, &control.tPrevErrorLed, &control.prevLEDerror);
  }
  else
  {
    digitalWrite(control.ledError, LOW);
  }
  //***************************************************************************
  //       SISTEMA DE CALEFACCION
  //***************************************************************************
  botonPermutaEstados(digitalRead(pinOnOff), 2000, 2000, &control.tPrevCambioOnOff, &control.estadoCalefaccion, Off, On);
  botonPermutaEstados(digitalRead(pinViaje), 3000, 3000, &control.tPrevCambioViaje, &control.estadoCalefaccion, control.estadoAnteriorViaje, Viaje);

  switch (control.estadoCalefaccion)
  {
  case Off:
    cerradoSistema(&control);
    control.estadoAnteriorViaje = control.estadoCalefaccion;
    break;

  case On:
    if (control.alimentacion.estadoUPS == estadosAlimentacion::ALIMENTACION_OK)
    {
      // Control Zona 1
      control.pisos[0].necesitaCalefaccion = controlHisteresis(control.pisos[0].temperaturaObjetivo, control.pisos[0].temperatura, control.pisos[0].histeresis, &control.pisos[0].necesitaCalefaccion);
      if (control.pisos[0].necesitaCalefaccion)
      {
        if (control.pisos[0].valvula != estadosValvula::Abierto)
        {
          activacionElectrovalvula(control.pisos[0].pinValvula, tactual, &control.pisos[0].tPrevValvula, 1000, &control.pisos[0].valvula, &control.pisos[0].valvulaAnterior);
        }
      }
      else
      {
        if (control.pisos[0].valvula != estadosValvula::Cerrado)
        {
          activacionElectrovalvula(control.pisos[0].pinValvula, tactual, &control.pisos[0].tPrevValvula, 1000, &control.pisos[0].valvula, &control.pisos[0].valvulaAnterior);
        }
      }
      // Control Zona 2
      control.pisos[1].necesitaCalefaccion = controlHisteresis(control.pisos[1].temperaturaObjetivo, control.pisos[1].temperatura, control.pisos[1].histeresis, &control.pisos[1].necesitaCalefaccion);
      if (control.pisos[1].necesitaCalefaccion)
      {
        if (control.pisos[1].valvula != estadosValvula::Abierto)
        {
          activacionElectrovalvula(control.pisos[1].pinValvula, tactual, &control.pisos[1].tPrevValvula, 1000, &control.pisos[1].valvula, &control.pisos[1].valvulaAnterior);
        }
      }
      else
      {
        if (control.pisos[1].valvula != estadosValvula::Cerrado)
        {
          activacionElectrovalvula(control.pisos[1].pinValvula, tactual, &control.pisos[1].tPrevValvula, 1000, &control.pisos[1].valvula, &control.pisos[1].valvulaAnterior);
        }
      }
      // Encender Apagar calefaccion principal.
      if (control.pisos[0].necesitaCalefaccion || control.pisos[1].necesitaCalefaccion)
      {
        if (control.valvulaPrincipal != estadosValvula::Abierto)
        {
          activacionElectrovalvula(control.pinPrincipal, tactual, &control.tPrevValvula, 1000, &control.valvulaPrincipal, &control.valvulaPrincipalAnterior);
        }
        if (control.bombaPrincipal == 0)
          control.bombaPrincipal = 1;
        if(control.temperaturaAcumulador >= control.temperaturaDisparoCaldera)
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
    break;

  case Viaje:
    if (control.alimentacion.estadoUPS == estadosAlimentacion::ALIMENTACION_OK)
    {
    }
    else // CIERRE POR ERRORES
    {
      cerradoSistema(&control);
    }
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
  }
}