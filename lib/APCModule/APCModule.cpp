#include <kroner_config.h>
#include "APCModule.h"

#ifndef DEBUG_PRINT
  #define DEBUG_PRINT(x) Serial.print(x)
#endif
#ifndef DEBUG_PRINTLN
  #define DEBUG_PRINTLN(x) Serial.println(x)
#endif

APCModule::APCModule(HardwareSerial &serial, int setPin) : serial(serial), hwSerial(&serial), swSerial(nullptr), setPin(setPin), rxPin(-1), txPin(-1) {
}

APCModule::APCModule(SoftwareSerial &serial, int setPin) : serial(serial), hwSerial(nullptr), swSerial(&serial), setPin(setPin), rxPin(-1), txPin(-1) {
}

APCModule::APCModule(HardwareSerial &serial, int setPin, int rxPin, int txPin)
    : serial(serial), hwSerial(&serial), swSerial(nullptr), setPin(setPin), rxPin(rxPin), txPin(txPin) {
}

void APCModule::beginSerial(int baudarate) {
  if (hwSerial != nullptr) {
    // En ESP32 podemos especificar RX/TX
    if (rxPin >= 0 && txPin >= 0) {
      hwSerial->begin(baudarate, SERIAL_8N1, rxPin, txPin);
    } else {
      hwSerial->begin(baudarate);
    }
  } else if (swSerial != nullptr) {
    swSerial->begin(baudarate);
  }
}

bool APCModule::tryReadConfig(String &out, unsigned long waitMs) {
  out = "";
  // Entrar en modo CONFIG
  digitalWrite(setPin, LOW);
  delay(1000);

  // Limpiar buffer de entrada
  while (serial.available()) {
    serial.read();
  }

  serial.println("RD");
  delay(100);

  unsigned long start = millis();
  while (millis() - start < waitMs) {
    if (serial.available()) {
      char c = (char)serial.read();
      out += c;
    }
  }

  // Volver a modo operación
  digitalWrite(setPin, HIGH);
  delay(200);

  return out.length() > 0;
}

void APCModule::init(int baudarate, int maxSetTimeOut) {
  DEBUG_PRINTLN("=================================");
  DEBUG_PRINTLN("Iniciando módulo APC220...");
  // Configurar pin SET
  pinMode(setPin, OUTPUT);
  digitalWrite(setPin, HIGH); // Modo operación por defecto

  // Iniciar directamente al baudrate indicado
  if (hwSerial) hwSerial->end();
  else if (swSerial) swSerial->end();
  delay(50);
  beginSerial(baudarate);
  serial.setTimeout(maxSetTimeOut);

  // Limpiar buffer de entrada
  while (serial.available()) {
    serial.read();
  }
}

void APCModule::setSettings(String ACPConfig){
  // Normalizar entrada: aceptar "PARA ...", "WR ..." o solo parámetros
  String in = ACPConfig; in.trim();
  String params = in;
  if (in.startsWith("PARA ")) {
    params = in.substring(5); // quitar "PARA "
  } else if (in.startsWith("WR ")) {
    params = in.substring(3);
  }
  params.trim();
  String toSend = String("WR ") + params; // comando real a enviar

  // Entrar en modo CONFIG
  digitalWrite(setPin, LOW);
  delay(10);

  // Limpiar buffer de entrada
  while (serial.available()) serial.read();

  // Enviar comando
  serial.println(toSend);

  // Leer respuesta con timeout
  String response;
  unsigned long start = millis();
  while (millis() - start < 600) { // un poco más de margen
    while (serial.available()) {
      response += (char)serial.read();
    }
  }

  // Salir a modo operación
  digitalWrite(setPin, HIGH);
  delay(200);

  // Validar respuesta: suele ser "PARA <freq> <rfRate> <power> <uart> <parity>"
  String expected = String("PARA ") + params;
  response.trim();
  if (response.startsWith(expected)) {
    Serial.println("APCModule: Configuración correcta");
  } else {
    Serial.print("APCModule: Respuesta inesperada: ");
    Serial.println(response);
    Serial.println("APCModule: Configuración puede no haberse aplicado");
  }
  DEBUG_PRINTLN("=================================");
}

String APCModule::getSettings(){
  String config;
  if (tryReadConfig(config, 500)) {
    return config;
  }
  return String("");
}
