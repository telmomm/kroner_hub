#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Wire.h>
#include <Keypad.h>

// FIRMWARE VERSION INFORMATION
#define FIRMWARE_VERSION "1.0.0"
#define FIRMWARE_BUILD_DATE __DATE__
#define FIRMWARE_BUILD_TIME __TIME__
#define DEVICE_MODEL "Kroner-Hub-v1"

// DEBUG MODE
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif


BLEService pulsadorService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic pulsadorCharacteristic("b444ea9a-a1b8-11ee-8c90-0242ac120002", BLENotify | BLERead, 40);
BLECharacteristic firmwareCharacteristic("c555fa9a-a1b8-11ee-8c90-0242ac120003", BLERead | BLEWrite, 50);

#define F1PIN 22
#define F2PIN 21
#define F3PIN 19
#define INPUT1PIN 32
#define INPUT2PIN 33
#define INPUT3PIN 27
#define INPUT4PIN 14
#define INPUT5PIN 12
#define INPUT6PIN 13
#define INPUT7PIN 15
#define INPUT8PIN 4
#define INPUT9PIN 5
#define INPUT10PIN 18

volatile uint32_t F1, F2, F3;

uint32_t message[4];

const uint32_t debounceTime = 200; // Tiempo de rebote en milisegundos
const uint32_t switchDebounceTime = 100; 
volatile uint32_t lastInterruptTimeF1 = 0;
volatile uint32_t lastInterruptTimeF2 = 0;
volatile uint32_t lastInterruptTimeF3 = 0;
bool newInputValue = false;

enum ButtonType {BOOL, TIMESTAMP};

struct Button {
  const char* name;
  ButtonType type;
  uint32_t timestamp;
  bool pressed;
};

const byte rowsCount = 3;
const byte columsCount = 3;

String keyNames[rowsCount][columsCount] = {
  {"Inicio", "Carrera", "Pausa"},
  {"Fin", "6 Segundos", "4 Puntos"},
  {"Reset", "Eliminado", "Extra"}
};

char keys[rowsCount][columsCount] = {
  { 'I','C','P'},
  { 'F','6','4'},
  { 'R','E','X'}
};

byte rowPins[rowsCount] = {INPUT4PIN, INPUT5PIN, INPUT6PIN};  
byte colPins[columsCount] = {INPUT1PIN, INPUT2PIN, INPUT3PIN}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rowsCount, columsCount);

// Función para crear y enviar información de firmware
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

// Función para procesar comandos recibidos por BLE
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

// Callback para cuando se escriba en la característica de firmware
void onFirmwareCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  String command = "";
  if (characteristic.valueLength() > 0) {
    for (int i = 0; i < characteristic.valueLength(); i++) {
      command += (char)characteristic.value()[i];
    }
    command.trim(); // Eliminar espacios en blanco
    processBLECommand(command);
  }
}

void handleInterruptF1() {
  uint32_t currentMillis = millis();
  if (currentMillis - lastInterruptTimeF1 > debounceTime) {
    F1 = currentMillis;
    lastInterruptTimeF1 = currentMillis;
    newInputValue = true;
  }
}

void handleInterruptF2() {
  uint32_t currentMillis = millis();
  if (currentMillis - lastInterruptTimeF2 > debounceTime) {
    F2 = currentMillis;
    lastInterruptTimeF2 = currentMillis;
    newInputValue = true;
  }
}

void handleInterruptF3() {
  uint32_t currentMillis = millis();
  if (currentMillis - lastInterruptTimeF3 > debounceTime) {
    F3 = currentMillis;
    lastInterruptTimeF3 = currentMillis;
    newInputValue = true;
  }
}
/*
float readBatteryLevel(){
  uint32_t raw = analogRead(VBATPIN);
  float voltage = raw * 3.3 / 4095.0;
  float maxVoltage = 3.6;
  float minVoltage = 2.55;
  float percentage = (voltage - minVoltage) / (maxVoltage - minVoltage) * 100;
  return percentage;
}
*/

//const uint32_t debounceTime = 200;
uint32_t lastPressedTime[3][3] = {0};
uint32_t lastInputTime [3] = {0};
bool inputState[3] = {false};
bool lastInputState[3] = {false};

//bool newInputValue = false;


void setup() {
  Serial.begin(115200);
  
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

  pinMode(F1PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(F1PIN), handleInterruptF1, RISING);

  pinMode(F2PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(F2PIN), handleInterruptF2, RISING);

  pinMode(F3PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(F3PIN), handleInterruptF3, RISING);

  analogReadResolution(12); 

  for (int i = 0; i < columsCount; i++) {
    pinMode(colPins[i], INPUT_PULLUP);  // Estabiliza la lectura
  }
  
  // Inicializar el BLE
  if (!BLE.begin()) {
    Serial.println("No se pudo iniciar BLE");
    while (1);
  }

  pinMode(INPUT7PIN, INPUT_PULLDOWN);

  // Configurar el nombre del dispositivo BLE
  BLE.setLocalName("Kroner-Hub");
  BLE.setDeviceName("Kroner-Hub");

  // Configurar el servicio BLE
  pulsadorService.addCharacteristic(pulsadorCharacteristic);
  pulsadorService.addCharacteristic(firmwareCharacteristic);
  BLE.addService(pulsadorService);
  
  // Configurar callback para cuando se escriba en la característica de firmware
  firmwareCharacteristic.setEventHandler(BLEWritten, onFirmwareCharacteristicWritten);


  // Iniciar el anuncio BLE
  BLE.setAdvertisingInterval(320); // 200 * 0.625 ms
  BLE.advertise();


}

void sendKeypadEvent(const String& name, uint32_t timestamp) {
  char payload[50];  // Asegúrate de que sea lo suficientemente grande
  snprintf(payload, sizeof(payload), "%s:%lu", name.c_str(), timestamp);

  Serial.println(payload);

  pulsadorCharacteristic.writeValue((uint8_t*)payload, strlen(payload));
}

bool lastSwitch1State = false;


void scanSwitches() {

  bool switch1State = digitalRead(INPUT7PIN);
  if (switch1State != lastSwitch1State) {
    lastSwitch1State = switch1State;
    char payload[50];  // Asegúrate de que sea lo suficientemente grande
    snprintf(payload, sizeof(payload), "Switch1:%s:%lu", switch1State ? "ON" : "OFF", millis());

  Serial.println(payload);
    DEBUG_PRINTLN(payload);
  }
  
}

bool input7State = false;

void scanSwitch(int inputNumber, int inputPin) {
  bool aux = digitalRead(inputPin);
  uint32_t now = millis();
  if (aux != inputState[inputNumber] && (now - lastInputTime[inputNumber] > switchDebounceTime)) {
    lastInputTime[inputNumber] = now;
    inputState[inputNumber] = aux;
    sendKeypadEvent(aux ? "Input" + String(inputNumber + 1) + " ON" : "Input" + String(inputNumber + 1) + " OFF", now);
  }
}

void scanKeypad() {
  char key = keypad.getKey();

  if (key) {
    uint32_t now = millis();

    for (int i = 0; i < rowsCount; i++) {
      for (int j = 0; j < columsCount; j++) {
        if (keys[i][j] == key) {
          if (now - lastPressedTime[i][j] > debounceTime) {
            lastPressedTime[i][j] = now;

            //Serial.print("Tecla presionada: ");
            //Serial.println(keyNames[i][j]);

            sendKeypadEvent(keyNames[i][j], now);
            return;
          }
        }
      }
    }
  }
}


void loop(){

  

  // Esperar a que se conecte un cliente BLE
  BLEDevice central = BLE.central();

  // Si se conecta un cliente BLE
  if (central) {
    DEBUG_PRINT("Conectado a: ");
    // Imprimir la dirección MAC del cliente BLE
    DEBUG_PRINTLN(central.address());
    DEBUG_PRINTLN(central.localName());
    DEBUG_PRINTLN(central.connected());
    
    // Enviar información de firmware al conectarse
    delay(1000); // Pequeña pausa para asegurar conexión estable
    sendFirmwareInfo();
    // Bucle para recibir comandos desde el cliente BLE
    while (central.connected()) {

      scanKeypad();

      scanSwitch(0, INPUT8PIN);

      

      if (newInputValue){
        bool status_switch1 = digitalRead(INPUT7PIN);
        message[0] = F1 + (status_switch1 ? 0x80000000 : 0);
        message[1] = F2;
        message[2] = F3;
        //message[3] = batterySOC;
        //message[3] = static_cast<uint32_t>(batterySOC * 100);

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

    // Desconectar el cliente BLE cuando ya no esté conectado
    Serial.println("Desconectado");
  }


}

