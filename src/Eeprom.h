#ifndef eeProto
#define eeProto
#include "GlobalStructures.h"
#include <Arduino.h>
#include <EEPROM.h>
void PresetFabrica(void);
void GuardarConfig(t_heating_system *sistema);
#endif