#ifndef SERIAL_FUNCTIONS_H
#define SERIAL_FUNCTIONS_H

#include <Arduino.h>
#include <APCModule.h>
#include "kroner_config.h"

// Variable global del módulo APC220
extern APCModule radio;

// Funciones de comunicación serial
void initAPC220();

#endif
