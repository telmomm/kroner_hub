#ifndef BLE_FUNCTIONS_H
#define BLE_FUNCTIONS_H

#include <Arduino.h>
#include <ArduinoBLE.h>

// Declaración de variables globales BLE (definidas en ble_functions.cpp)
extern BLEService pulsadorService;
extern BLECharacteristic pulsadorCharacteristic;
extern BLECharacteristic firmwareCharacteristic;
extern BLEService serialBridgeService;
extern BLECharacteristic serialBridgeWriteChar;

// Buffer para datos recibidos por BLE (para enviar al APC220)
extern volatile uint8_t bleMessageBuffer[255];
extern volatile int bleMessageLen;
extern volatile unsigned long bleMessageTime;
extern volatile bool bleMessageReady;

// Funciones BLE
void initBLE();
void sendFirmwareInfo();
void sendHelpInfo();
void processBLECommand(const String& command);
void onFirmwareCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic);
void onSerialBridgeWritten(BLEDevice central, BLECharacteristic characteristic);

#endif
