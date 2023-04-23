#include <Arduino.h>
#include <TimerOne.h>
#include "GlobalStructures.h"
#include "RelojConEstructuras.h"
#include "BlinkSinDelays.h"

void contarTiempo(void);
void shell(void);

t_heating_system control;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(contarTiempo);
  String hora = __TIME__;
  control.horaReal = StringToTiempo(hora);
}

void contarTiempo(void)
{
  control.horaReal.seconds += 1;
}

void loop() {
  // put your main code here, to run repeatedly:
  t_time horaActualCopy;
  noInterrupts();
  guardarTiempo(control.horaReal);
  horaActualCopy = control.horaReal;
  interrupts();
  
}