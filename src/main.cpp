#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Wire.h>
#include <Keypad.h>
#include <APCModule.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>


// FIRMWARE VERSION INFORMATION 
#define FIRMWARE_VERSION "1.0.1"
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

// Servicio BLE para puente serie (escritura -> Serial2)
BLEService serialBridgeService("12345678-1234-5678-1234-56789abcdef0");
BLECharacteristic serialBridgeWriteChar("12345678-1234-5678-1234-56789abcdef1", BLEWrite | BLEWriteWithoutResponse, 244);

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
// APC220 PINPUT
#define APC_RXPIN 16
#define APC_TXPIN 17
#define APC_SETPIN 23

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

// Instancia del módulo APC220 usando Serial2 (RX=16, TX=17) y SET=23
APCModule radio(Serial2, APC_SETPIN, APC_RXPIN, APC_TXPIN);

// Web Server
WebServer webServer(80);
DNSServer dnsServer;

// Buffer para datos recibidos por BLE (para clientes HTTP que hagan polling)
volatile uint8_t lastMessageBuffer[255];
volatile int lastMessageLen = 0;
volatile unsigned long lastMessageTime = 0;

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

// Callback: al escribir en el puente BLE, reenviar a Serial2 Y guardar en buffer para web
void onSerialBridgeWritten(BLEDevice central, BLECharacteristic characteristic) {
  int len = characteristic.valueLength();
  if (len <= 0) return;
  const uint8_t* data = characteristic.value();

  // Reenviar bytes crudos a Serial2 (UART del APC220)
  Serial2.write(data, len);
  Serial2.flush();

  // Guardar en buffer para clientes HTTP
  if (len <= 255) {
    memcpy((void*)lastMessageBuffer, data, len);
    lastMessageLen = len;
    lastMessageTime = millis();
  }

  // Log opcional por USB
  DEBUG_PRINT("BLE->UART (bytes): ");
  DEBUG_PRINTLN(len);
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

// Función para servir la página HTML desde LittleFS
void handleRoot() {
  if (LittleFS.exists("/index.html")) {
    File file = LittleFS.open("/index.html", "r");
    webServer.streamFile(file, "text/html");
    file.close();
  } else {
    webServer.send(404, "text/plain", "index.html no encontrado");
  }
}

// Captive Portal: redirigir cualquier ruta desconocida a la raíz
void handleNotFound() {
  // Si no es una petición a API, redirigir a la raíz
  String path = webServer.uri();
  if (!path.startsWith("/api/")) {
    webServer.sendHeader("Location", "http://192.168.4.1/", true);
    webServer.send(302, "text/plain", "");
  } else {
    webServer.send(404, "text/plain", "Not found");
  }
}

// API: obtener último mensaje recibido
void handleGetMessages() {
  char jsonResponse[512];
  
  // Convertir buffer a base64
  char base64Buffer[400];
  int base64Len = 0;
  if (lastMessageLen > 0) {
    // Implementación simple de base64
    const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i = 0;
    while (i < lastMessageLen) {
      uint8_t b1 = lastMessageBuffer[i++];
      uint8_t b2 = (i < lastMessageLen) ? lastMessageBuffer[i++] : 0;
      uint8_t b3 = (i < lastMessageLen) ? lastMessageBuffer[i++] : 0;
      
      base64Buffer[base64Len++] = alphabet[b1 >> 2];
      base64Buffer[base64Len++] = alphabet[((b1 & 0x03) << 4) | (b2 >> 4)];
      if (i - 1 < lastMessageLen) {
        base64Buffer[base64Len++] = alphabet[((b2 & 0x0F) << 2) | (b3 >> 6)];
      }
      if (i < lastMessageLen) {
        base64Buffer[base64Len++] = alphabet[b3 & 0x3F];
      }
    }
  }
  base64Buffer[base64Len] = '\0';
  
  snprintf(jsonResponse, sizeof(jsonResponse), 
    "{\"len\":%d,\"time\":%lu,\"data\":\"%s\"}",
    lastMessageLen, lastMessageTime, base64Buffer);
  
  webServer.send(200, "application/json", jsonResponse);
}

// API: enviar mensaje a Serial2
void handleSendMessage() {
  if (webServer.hasArg("plain")) {
    String msg = webServer.arg("plain");
    Serial2.write((uint8_t*)msg.c_str(), msg.length());
    Serial2.flush();
    webServer.send(200, "text/plain", "OK");
  } else {
    webServer.send(400, "text/plain", "No message");
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

  // Inicializar LittleFS
  if (!LittleFS.begin()) {
    DEBUG_PRINTLN("Error al inicializar LittleFS");
  } else {
    DEBUG_PRINTLN("LittleFS iniciado correctamente");
  }

  // Configurar WiFi AP
  DEBUG_PRINTLN("Iniciando WiFi AP...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Kroner", "");  // SSID "Kroner", sin contraseña
  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  DEBUG_PRINT("AP IP: ");
  DEBUG_PRINTLN(WiFi.softAPIP());

  // Configurar DNS para Captive Portal (redirigir todo a 192.168.4.1)
  dnsServer.start(53, "*", apIP);
  DEBUG_PRINTLN("DNS Server iniciado para Captive Portal");

  // Configurar Web Server
  webServer.on("/", handleRoot);
  webServer.on("/api/messages", handleGetMessages);
  webServer.on("/api/send", HTTP_POST, handleSendMessage);
  webServer.onNotFound(handleNotFound);  // Captive Portal redirect
  webServer.begin();
  DEBUG_PRINTLN("Web Server iniciado en puerto 80");

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
  // Configurar entradas discretas 7/8/9 con pull-up interno
  // Nota: GPIO4, GPIO5 y GPIO15 son "strapping pins" del ESP32; evitar forzarlos
  // en estados inválidos durante el arranque. Tras el boot, pueden usarse con cuidado.
  pinMode(INPUT7PIN, INPUT_PULLDOWN);
  pinMode(INPUT8PIN, INPUT_PULLDOWN);
  pinMode(INPUT9PIN, INPUT_PULLDOWN);
  
  pinMode(APC_SETPIN, OUTPUT);
  
  // Inicializar APC220 a baud fijo
  radio.init(9600, 500);

  // Aplicar configuración previa solicitada
  // Parámetros: 435000 (freq kHz), RF=3(9600), Power=9, UART=3(9600), Parity=0
  radio.setSettings("PARA 435000 3 9 3 0");

  // Leer configuración para verificar
  String resp = radio.getSettings();
  DEBUG_PRINTLN(resp);              // Espera: "PARA 435000 3 9 3 0"

  // Inicializar el BLE
  if (!BLE.begin()) {
    Serial.println("No se pudo iniciar BLE");
    while (1);
  }


  // Configurar el nombre del dispositivo BLE
  BLE.setLocalName("Kroner-Hub");
  BLE.setDeviceName("Kroner-Hub");

  // Configurar el servicio BLE
  pulsadorService.addCharacteristic(pulsadorCharacteristic);
  pulsadorService.addCharacteristic(firmwareCharacteristic);
  BLE.addService(pulsadorService);

  // Servicio de puente serie
  serialBridgeService.addCharacteristic(serialBridgeWriteChar);
  BLE.addService(serialBridgeService);
  
  // Configurar callback para cuando se escriba en la característica de firmware
  firmwareCharacteristic.setEventHandler(BLEWritten, onFirmwareCharacteristicWritten);
  // Callback para puente serie (escritura -> UART)
  serialBridgeWriteChar.setEventHandler(BLEWritten, onSerialBridgeWritten);


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



void scanSwitch(int inputNumber, int inputPin) {
  bool aux = digitalRead(inputPin);
  uint32_t now = millis();
  if (aux != inputState[inputNumber] && (now - lastInputTime[inputNumber] > switchDebounceTime)) {
    lastInputTime[inputNumber] = now;
    inputState[inputNumber] = aux;
    sendKeypadEvent(aux ? "Input" + String(inputNumber + 1) + " ON" : "Input" + String(inputNumber + 1) + " OFF", now);
  }
}

void sendInitialSwitchState(int inputNumber, int inputPin) {
  bool aux = digitalRead(inputPin);
  uint32_t now = millis();
  inputState[inputNumber] = aux;
  lastInputTime[inputNumber] = now;
  sendKeypadEvent(aux ? "Input" + String(inputNumber + 1) + " ON" : "Input" + String(inputNumber + 1) + " OFF", now);
  DEBUG_PRINT("Estado inicial Switch ");
  DEBUG_PRINT(inputNumber + 1);
  DEBUG_PRINT(": ");
  DEBUG_PRINTLN(aux ? "ON" : "OFF");
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
  // Procesar peticiones HTTP
  webServer.handleClient();
  
  // Procesar peticiones DNS para Captive Portal
  dnsServer.processNextRequest();

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

    // Enviar estado inicial de todos los switches
    DEBUG_PRINTLN("Enviando estado inicial de switches...");
    sendInitialSwitchState(0, INPUT7PIN);
    sendInitialSwitchState(1, INPUT8PIN);
    sendInitialSwitchState(2, INPUT9PIN);

    // Bucle para recibir comandos desde el cliente BLE
    while (central.connected()) {
      // Procesar peticiones HTTP incluso durante conexión BLE
      webServer.handleClient();

      scanKeypad();

      scanSwitch(0, INPUT7PIN);
      scanSwitch(1, INPUT8PIN);
      scanSwitch(2, INPUT9PIN);

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

