#include <Arduino.h>
#include <ArduinoBLE.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Keypad.h>


// DEBUG MODE
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif


#define SDA_PIN 17  // Pin SDA (Serial Data)
#define SCL_PIN 18  // Pin SCL (Serial Clock)
#define RST_PIN 21  // Pin RST (Reset)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, RST_PIN, SCL_PIN, SDA_PIN);


BLEService pulsadorService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic pulsadorCharacteristic("b444ea9a-a1b8-11ee-8c90-0242ac120002", BLENotify | BLERead, 40);

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
//bool newInputValue = false;


void setup() {
  Serial.begin(115200);

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

  pinMode(INPUT8PIN, INPUT);

  // Configurar el nombre del dispositivo BLE
  BLE.setLocalName("Kroner-Hub");
  BLE.setDeviceName("Kroner-Hub");

  // Configurar el servicio BLE
  pulsadorService.addCharacteristic(pulsadorCharacteristic);
  BLE.addService(pulsadorService);


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
    Serial.print("Conectado a: ");
    // Imprimir la dirección MAC del cliente BLE
    Serial.println(central.address());
    Serial.println(central.localName());
    Serial.println(central.connected());
    // Bucle para recibir comandos desde el cliente BLE
    while (central.connected()) {

      scanKeypad();

      if (newInputValue){
        message[0] = F1;
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

