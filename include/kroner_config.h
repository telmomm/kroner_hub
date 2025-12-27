#ifndef KRONER_CONFIG_H
#define KRONER_CONFIG_H

#include <Arduino.h>

// =============================
// Firmware info
// =============================
#define DEVICE_MODEL "Kroner-Hub-v1"
#define FIRMWARE_VERSION "1.0.3"
#define FIRMWARE_BUILD_DATE __DATE__
#define FIRMWARE_BUILD_TIME __TIME__

// =============================
// Debug
// =============================
#define DEBUG 1
#if DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// =============================
// Serial baudrates
// =============================
#define SERIAL_BAUD_USB 115200
#define RADIO_UART_BAUD 9600
#define RADIO_AIR_BAUD 500

// =============================
// Radio settings (APC220)
// =============================
#define RADIO_SETTINGS_STRING "PARA 435000 3 9 3 0"

// =============================
// WiFi / Captive Portal
// =============================
#define WIFI_AP_SSID "Kroner"
#define WIFI_AP_PASS ""
#define WIFI_AP_IP0 192
#define WIFI_AP_IP1 168
#define WIFI_AP_IP2 4
#define WIFI_AP_IP3 1
#define WIFI_AP_MASK0 255
#define WIFI_AP_MASK1 255
#define WIFI_AP_MASK2 255
#define WIFI_AP_MASK3 0

// =============================
// Pin mapping
// =============================
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

// APC220 pins
#define APC_RXPIN 16
#define APC_TXPIN 17
#define APC_SETPIN 23

#endif // KRONER_CONFIG_H
