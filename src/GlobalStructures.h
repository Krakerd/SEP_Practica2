#ifndef globalStruct
#define globalStruct
#include <Arduino.h>
typedef struct t_time
{
  int hour;
  int minuts;
  int seconds;
};
typedef struct t_solar_collector
{
  float temperatura;
  short int bomba;
  short int valvula;
  short int valvulaAnterior;
  uint8_t pinCollector;
};
typedef struct t_heating_room
{
  t_time horaEncendido;
  float histeresis;
  float sensorRangoAlto;
  float sensorRangoBajo;
  short int valvula;
  short int valvulaAnterior;
  uint8_t pinCuarto;
};
typedef struct t_heating_floor{
  t_heating_room cuartos[2];
  float temperaturaObjetivo;
  short int valvula;
  short int valvulaAnterior;
  uint8_t pinZona;
};
typedef struct t_heating_system{
  t_time horaReal;
  t_heating_floor pisos[2];
  t_solar_collector colectores[1];
  short int bombaPrincipal;
  float temperaturaAcumulador;
  short int valvulaPrincipal;
  short int valvulaPrincipalAnterior;
  short int calderaOnOff;
  uint8_t pinPrincipal;
  uint8_t pinCaldera;
};

#endif