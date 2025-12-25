#include <Arduino.h>
#include "kroner_config.h"
#include "ble_functions.h"
#include "webserver_functions.h"
#include "input_functions.h"
#include "serial_functions.h"

uint32_t message[4];

void setup() {
  Serial.begin(SERIAL_BAUD_USB);
  
  // Mostrar información de firmware al inicio
  DEBUG_PRINTLN("=================================");
  DEBUG_PRINT("Device: ");
  DEBUG_PRINTLN(DEVICE_MODEL);
  DEBUG_PRINT("Firmware Version: ");
  DEBUG_PRINTLN(FIRMWARE_VERSION);
  DEBUG_PRINT("Build Date: ");
  DEBUG_PRINT(FIRMWARE_BUILD_DATE);
  DEBUG_PRINT(" ");
  DEBUG_PRINTLN(FIRMWARE_BUILD_TIME);
  DEBUG_PRINTLN("=================================");

  // Inicializar módulos
  initWiFiAP();
  initWebServer();
  initInputs();
  initAPC220();
  initBLE();

  analogReadResolution(12);
}

void loop() {
  // Procesar peticiones HTTP y DNS
  webServer.handleClient();
  dnsServer.processNextRequest();

  // Esperar a que se conecte un cliente BLE
  BLEDevice central = BLE.central();

  if (central) {
    DEBUG_PRINT("Conectado a: ");
    DEBUG_PRINTLN(central.address());
    DEBUG_PRINTLN(central.localName());
    DEBUG_PRINTLN(central.connected());
    
    // Enviar información de firmware al conectarse
    delay(1000);
    sendFirmwareInfo();

    // Enviar estado inicial de switches
    DEBUG_PRINTLN("Enviando estado inicial de switches...");
    sendInitialSwitchState(0, INPUT7PIN);
    sendInitialSwitchState(1, INPUT8PIN);
    sendInitialSwitchState(2, INPUT9PIN);

    // Bucle mientras está conectado BLE
    while (central.connected()) {
      // Procesar peticiones HTTP
      webServer.handleClient();

      // Escanear entradas
      scanKeypad();
      scanSwitch(0, INPUT7PIN);
      scanSwitch(1, INPUT8PIN);
      scanSwitch(2, INPUT9PIN);

      // Enviar valores de F1, F2, F3 si hay cambios
      if (newInputValue) {
        message[0] = F1;
        message[1] = F2;
        message[2] = F3;

        Serial.print("F1: ");
        Serial.print(F1);
        Serial.print(" - F2: ");
        Serial.print(F2);
        Serial.print(" - F3: ");
        Serial.print(F3);
        Serial.println();

        pulsadorCharacteristic.writeValue((uint8_t*)message, sizeof(message));
        newInputValue = false;
      }
    }

    Serial.println("Desconectado");
  }
}
