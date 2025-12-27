#include "kroner_config.h"
#include "ble_functions.h"

// Servicios y características BLE
BLEService pulsadorService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic pulsadorCharacteristic("b444ea9a-a1b8-11ee-8c90-0242ac120002", BLENotify | BLERead, 40);
BLECharacteristic firmwareCharacteristic("c555fa9a-a1b8-11ee-8c90-0242ac120003", BLERead | BLEWrite, 50);

BLEService serialBridgeService("12345678-1234-5678-1234-56789abcdef0");
BLECharacteristic serialBridgeWriteChar("12345678-1234-5678-1234-56789abcdef1", BLEWrite | BLEWriteWithoutResponse, 244);

// Buffer para datos recibidos por BLE (para enviar al APC220)
volatile uint8_t bleMessageBuffer[255];
volatile int bleMessageLen = 0;
volatile unsigned long bleMessageTime = 0;
volatile bool bleMessageReady = false;

void initBLE() {
  if (!BLE.begin()) {
    Serial.println("No se pudo iniciar BLE");
    while (1);
  }

  // Configurar el nombre del dispositivo BLE
  BLE.setLocalName("Kroner-Hub");
  BLE.setDeviceName("Kroner-Hub");

  // Configurar el servicio BLE de pulsadores
  pulsadorService.addCharacteristic(pulsadorCharacteristic);
  pulsadorService.addCharacteristic(firmwareCharacteristic);
  BLE.addService(pulsadorService);

  // Servicio de puente serie
  serialBridgeService.addCharacteristic(serialBridgeWriteChar);
  BLE.addService(serialBridgeService);
  
  // Configurar callbacks
  firmwareCharacteristic.setEventHandler(BLEWritten, onFirmwareCharacteristicWritten);
  serialBridgeWriteChar.setEventHandler(BLEWritten, onSerialBridgeWritten);

  // Iniciar el anuncio BLE
  BLE.setAdvertisingInterval(320); // 200 * 0.625 ms
  BLE.advertise();

  DEBUG_PRINTLN("BLE iniciado correctamente");
}

void sendFirmwareInfo() {
  char firmwareInfo[100];
  snprintf(firmwareInfo, sizeof(firmwareInfo), 
           "FW:%s|Model:%s|Build:%s %s", 
           FIRMWARE_VERSION, 
           DEVICE_MODEL, 
           FIRMWARE_BUILD_DATE, 
           FIRMWARE_BUILD_TIME);
  
  DEBUG_PRINT("Enviando info firmware: ");
  DEBUG_PRINTLN(firmwareInfo);
  
  firmwareCharacteristic.writeValue((uint8_t*)firmwareInfo, strlen(firmwareInfo));
}

void sendHelpInfo() {
  char helperInfo[200];
  snprintf(helperInfo, sizeof(helperInfo), 
           "Help | FW Version | RESET");
  
  DEBUG_PRINT("Enviando info ayuda: ");
  DEBUG_PRINTLN(helperInfo);
  firmwareCharacteristic.writeValue((uint8_t*)helperInfo, strlen(helperInfo));
}

void processBLECommand(const String& command) {
  DEBUG_PRINT("Comando recibido: ");
  DEBUG_PRINTLN(command);
  
  if (command == "FW Version" || command == "FW_VERSION" || command == "fw version") {
    sendFirmwareInfo();
  }
  else if (command == "RESET") {
    DEBUG_PRINTLN("Reiniciando dispositivo...");
    ESP.restart();
  }
  else if (command == "HELP" || command == "Help" || command == "help") {
    DEBUG_PRINTLN("Comandos disponibles:");
    DEBUG_PRINTLN(" - FW Version");
    DEBUG_PRINTLN(" - RESET");
    DEBUG_PRINTLN(" - HELP");
    sendHelpInfo();
  }
  else {
    DEBUG_PRINT("Comando no reconocido: ");
    DEBUG_PRINTLN(command);
  }
}

void onFirmwareCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  String command = "";
  if (characteristic.valueLength() > 0) {
    for (int i = 0; i < characteristic.valueLength(); i++) {
      command += (char)characteristic.value()[i];
    }
    command.trim();
    processBLECommand(command);
  }
}

void onSerialBridgeWritten(BLEDevice central, BLECharacteristic characteristic) {
  int len = characteristic.valueLength();
  if (len <= 0 || len > 255) return;
  const uint8_t* data = characteristic.value();

  // Guardar datos en buffer para que la tarea los procese
  memcpy((void*)bleMessageBuffer, data, len);
  bleMessageLen = len;
  bleMessageTime = millis();
  bleMessageReady = true;

  DEBUG_PRINT("BLE mensaje recibido (bytes): ");
  DEBUG_PRINTLN(len);
}
