#include <Arduino.h>
#include <TimerOne.h>
#include "GlobalStructures.h"
#include "ControlFunctions.h"
#include "RelojConEstructuras.h"
#include "BlinkSinDelays.h"

void contarTiempo(void);
void shell(void);

t_heating_system control;

void setup() {
  //***************************************************************************
  //        INICIAR SERIAL Y INTERRUPCION
  //***************************************************************************
  Serial.begin(115200);
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(contarTiempo);

  //***************************************************************************
  //        PINMODES
  //***************************************************************************
  for(int i = 8; i<=13; i++) pinMode(i,OUTPUT);
  for(int i = 2; i<=4; i++) pinMode(i,INPUT_PULLUP);

  //***************************************************************************
  //        PUESTA EN HORA DEL RELOJ
  //***************************************************************************
  String hora = __TIME__;
  control.horaReal = StringToTiempo(hora);

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
  control.sensorAcumulador.RangoAlto = 80.0;
  control.sensorAcumulador.RangoBajo = -5.0;
  control.colectores[0].sensorT.pin = A3;
  control.colectores[0].sensorT.RangoBajo = -5.0;
  control.colectores[0].sensorT.RangoAlto = 80.0;
  
  //***************************************************************************
  //        PRESET ZONAS
  //***************************************************************************
  control.pisos[0].sensorT.pin = A1;
  control.pisos[0].pinValvula = 9;
  control.pisos[0].temperaturaObjetivo = 20.0; // temperatura objetivo debe ir a eeprom
  control.pisos[0].histeresis = 1.0; // eeprom
  control.pisos[0].sensorT.RangoAlto = 80.0; // eeprom
  control.pisos[0].sensorT.RangoBajo = -5.0; // eeprom
  
  control.pisos[1].sensorT.pin = A5;
  control.pisos[1].pinValvula = 11;
  control.pisos[1].temperaturaObjetivo = 20.0; // temperatura objetivo debe ir a eeprom
  control.pisos[1].histeresis = 1.0; // eeprom
  control.pisos[1].sensorT.RangoAlto = 80.0; // eeprom
  control.pisos[1].sensorT.RangoBajo = -5.0; // eeprom
}

void contarTiempo(void)
{
  control.horaReal.seconds += 1;
}

void loop() {
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
  control.pisos[0].temperatura = mapFloat(analogRead(control.pisos[0].sensorT.pin),0.0, 1023.0,control.pisos[0].sensorT.RangoBajo,control.pisos[0].sensorT.RangoAlto);
  control.pisos[1].temperatura = mapFloat(analogRead(control.pisos[1].sensorT.pin),0.0, 1023.0,control.pisos[1].sensorT.RangoBajo,control.pisos[1].sensorT.RangoAlto);
  control.temperaturaAcumulador = mapFloat(analogRead(control.sensorAcumulador.pin),0.0, 1023.0,control.sensorAcumulador.RangoBajo, control.sensorAcumulador.RangoAlto);
  control.colectores[0].temperatura = mapFloat(analogRead(control.colectores[0].sensorT.pin),0.0, 1023.0,control.colectores[0].sensorT.RangoBajo,control.colectores[0].sensorT.RangoAlto);
  //***************************************************************************
  //       LECTURA Y CONTROL UPS
  //***************************************************************************
  control.alimentacion.lecturaSensor = mapFloat(analogRead(control.alimentacion.pinUPS), 0.0, 1023.0, 0.0, 5.0);
  control.alimentacion.voltajeAlimentacion = mapFloat(control.alimentacion.lecturaSensor, 0.0, 4.0, 0.0, 12.0);
  control.alimentacion.estadoUPS = estadoUPS(control.alimentacion.voltajeAlimentacion,control.alimentacion.margenVoltaje,control.alimentacion.voltajeDeseado);

  Imprimir("TZona1",control.pisos[0].temperatura);
  Imprimir("TZona2",control.pisos[1].temperatura);
  Imprimir("TColector",control.colectores[0].temperatura);
  Imprimir("TAcumulador",control.temperaturaAcumulador);
}