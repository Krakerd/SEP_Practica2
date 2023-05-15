#ifndef eeProto
#define eeProto
#include "GlobalStructures.h"
#include <Arduino.h>
#include <EEPROM.h>
void PresetFabrica(void);
void De_eeprom_a_structura_fabrica(t_heating_system *C);
void GuardarConfig(t_heating_system *sistema);
#endif