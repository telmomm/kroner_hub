#include "kroner_config.h"
#include "input_functions.h"
#include "ble_functions.h"

// Variables globales de interrupciones
volatile uint32_t F1, F2, F3;
volatile bool newInputValue = false;
const uint32_t debounceTime = 500;      // valor previo estable para keypad
const uint32_t switchDebounceTime = 100;
volatile uint32_t lastInterruptTimeF1 = 0;
volatile uint32_t lastInterruptTimeF2 = 0;
volatile uint32_t lastInterruptTimeF3 = 0;

// Variables de switches
uint32_t lastInputTime[3] = {0};
bool inputState[3] = {false};
bool lastInputState[3] = {false};

// Variables de keypad
const byte rowsCount = 3;
const byte columsCount = 3;

String keyNames[rowsCount][columsCount] = {
  {"Inicio", "Carrera", "Pausa"},
  {"Fin", "6 Segundos", "4 Puntos"},
  {"Reset", "Eliminado", "Perilla"}
};

char keys[rowsCount][columsCount] = {
  { 'I','C','P'},
  { 'F','6','4'},
  { 'R','E','X'}
};

byte rowPins[rowsCount] = {INPUT4PIN, INPUT5PIN, INPUT6PIN};
byte colPins[columsCount] = {INPUT1PIN, INPUT2PIN, INPUT3PIN};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rowsCount, columsCount);

uint32_t lastPressedTime[3][3] = {0};

// Funciones de interrupciones
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

void initInputs() {
  // Configurar pines F con interrupciones
  pinMode(F1PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(F1PIN), handleInterruptF1, FALLING);

  pinMode(F2PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(F2PIN), handleInterruptF2, FALLING);

  pinMode(F3PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(F3PIN), handleInterruptF3, FALLING);

  // Configurar columnas del keypad
  for (int i = 0; i < columsCount; i++) {
    pinMode(colPins[i], INPUT_PULLUP);
  }

  // Afinar tiempos del Keypad
  keypad.setDebounceTime(10);  // ms (valor previo)
  keypad.setHoldTime(200);     // ms

  // Configurar entradas discretas 7/8/9
  pinMode(INPUT7PIN, INPUT_PULLDOWN);
  pinMode(INPUT8PIN, INPUT_PULLDOWN);
  pinMode(INPUT9PIN, INPUT_PULLDOWN);
}

void sendKeypadEvent(const String& name, uint32_t timestamp) {
  char payload[50];
  snprintf(payload, sizeof(payload), "%s:%lu", name.c_str(), timestamp);

  DEBUG_PRINTLN(payload);
  pulsadorCharacteristic.writeValue((uint8_t*)payload, strlen(payload));
}

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
    DEBUG_PRINT("🔘 Keypad detected: ");
    DEBUG_PRINT(key);
    DEBUG_PRINT(" | Millis: ");
    DEBUG_PRINTLN(millis());
    
    uint32_t now = millis();

    for (int i = 0; i < rowsCount; i++) {
      for (int j = 0; j < columsCount; j++) {
        if (keys[i][j] == key) {
          uint32_t timeSinceLastPress = now - lastPressedTime[i][j];
          
          // Debug: mostrar tiempo desde última pulsación
          DEBUG_PRINT("  Time since last press: ");
          DEBUG_PRINT(timeSinceLastPress);
          DEBUG_PRINT(" ms (debounce: ");
          DEBUG_PRINT(debounceTime);
          DEBUG_PRINTLN(" ms)");
          
          if (timeSinceLastPress > debounceTime) {
            lastPressedTime[i][j] = now;
            DEBUG_PRINT("  ✓ VALID - Sending: ");
            DEBUG_PRINTLN(keyNames[i][j]);
            sendKeypadEvent(keyNames[i][j], now);
            return;
          } else {
            DEBUG_PRINTLN("  ✗ Ignored (debouncing)");
          }
        }
      }
    }
  }
}
