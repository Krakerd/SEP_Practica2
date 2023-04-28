#ifndef globalStruct
#define globalStruct
#include <Arduino.h>
enum estadosValvula
{
  Cerrado = 0,
  Abierto = 1,
  Cambiando = 2
};
enum estadosAlimentacion
{
  ALIMENTACION_OK,
  ALIMENTACION_SOBRE,
  ALIMENTACION_DESC
};
enum estadosCalefaccion
{
  Off,
  On,
  Viaje
};

typedef struct t_time
{
  unsigned short int hour;
  unsigned short int minuts;
  unsigned short int seconds;
};

typedef struct t_sensorTemp
{
  float RangoAlto;
  float RangoBajo;
  uint8_t pin;
};

typedef struct t_solar_collector
{
  t_sensorTemp sensorT;
  float temperatura;
  float temperaturaVaciado;
  short int bomba;
  estadosValvula valvula;
  estadosValvula valvulaAnterior;
  unsigned long tPrevValvula;
  uint8_t pinValvula;
};

/*typedef struct t_heating_room
{
  t_time horaEncendido;
  short int valvula;
  short int valvulaAnterior;
  uint8_t pinCuarto;
};*/

typedef struct t_heating_floor
{
  // t_heating_room cuartos[2];
  t_sensorTemp sensorT;
  float temperaturaObjetivo;
  float temperatura;
  float histeresis;
  bool necesitaCalefaccion;
  estadosValvula valvula;
  estadosValvula valvulaAnterior;
  unsigned long tPrevValvula;
  uint8_t pinValvula;
};

typedef struct t_ups
{
  estadosAlimentacion estadoUPS;
  unsigned long prevMillis;
  uint8_t pinUPS;
  float lecturaSensor;
  float voltajeAlimentacion;
  float voltajeDeseado;
  float margenVoltaje;
};

typedef struct t_heating_system
{
  t_time horaReal;
  t_heating_floor pisos[2];
  t_solar_collector colectores[1];
  t_ups alimentacion;
  unsigned short int bombaPrincipal;
  float temperaturaAcumulador;
  float temperaturaAcumuladorError;
  t_sensorTemp sensorAcumulador;
  estadosValvula valvulaPrincipal;
  estadosValvula valvulaPrincipalAnterior;
  estadosCalefaccion estadoCalefaccion;
  estadosCalefaccion estadoAnteriorViaje;
  unsigned long tPrevValvula;
  unsigned long tPrevCambioOnOff;
  unsigned long tPrevCambioViaje;
  unsigned long tPrevErrorLed;
  bool prevLEDerror;
  uint8_t pinPrincipal;
  uint8_t pinCaldera;
  uint8_t ledError;
};

#endif