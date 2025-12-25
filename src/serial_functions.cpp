#include "kroner_config.h"
#include "serial_functions.h"

// Instancia del módulo APC220
APCModule radio(Serial2, APC_SETPIN, APC_RXPIN, APC_TXPIN);

void initAPC220() {
  pinMode(APC_SETPIN, OUTPUT);
  
  // Inicializar APC220 con baudios configurables
  radio.init(RADIO_UART_BAUD, RADIO_AIR_BAUD);

  // Aplicar configuración: 435000 kHz, RF=3(9600), Power=9, UART=3(9600), Parity=0
  radio.setSettings(RADIO_SETTINGS_STRING);

  // Leer configuración para verificar
  String resp = radio.getSettings();
  DEBUG_PRINTLN(resp);
}
