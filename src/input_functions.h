#ifndef INPUT_FUNCTIONS_H
#define INPUT_FUNCTIONS_H

#include <Arduino.h>
#include <Keypad.h>
#include "kroner_config.h"

// Variables globales de interrupciones
extern volatile uint32_t F1, F2, F3;
extern volatile bool newInputValue;
extern volatile uint32_t lastInterruptTimeF1;
extern volatile uint32_t lastInterruptTimeF2;
extern volatile uint32_t lastInterruptTimeF3;

// Variables de switches
extern uint32_t lastInputTime[3];
extern bool inputState[3];
extern bool lastInputState[3];

// Variables de keypad
extern const byte rowsCount;
extern const byte columsCount;
extern String keyNames[3][3];
extern char keys[3][3];
extern byte rowPins[3];
extern byte colPins[3];
extern Keypad keypad;
extern uint32_t lastPressedTime[3][3];

// Constantes
extern const uint32_t debounceTime;
extern const uint32_t switchDebounceTime;

// Funciones de interrupciones
void handleInterruptF1();
void handleInterruptF2();
void handleInterruptF3();

// Funciones de entrada
void initInputs();
void scanSwitch(int inputNumber, int inputPin);
void sendInitialSwitchState(int inputNumber, int inputPin);
void scanKeypad();
void sendKeypadEvent(const String& name, uint32_t timestamp);

#endif
