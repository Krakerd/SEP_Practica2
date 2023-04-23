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

typedef struct t_time
{
  int hour;
  int minuts;
  int seconds;
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
  short int valvula;
  short int valvulaAnterior;
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
  short int valvula;
  short int valvulaAnterior;
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
  short int bombaPrincipal;
  float temperaturaAcumulador;
  t_sensorTemp sensorAcumulador;
  short int valvulaPrincipal;
  short int valvulaPrincipalAnterior;
  short int calderaOnOff;
  uint8_t pinPrincipal;
  uint8_t pinCaldera;
  uint8_t ledError;
};

#endif