#ifndef WEBSERVER_FUNCTIONS_H
#define WEBSERVER_FUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <DNSServer.h>
#include <LittleFS.h>

// Declaración de variables globales WebServer
extern WebServer webServer;
extern DNSServer dnsServer;
extern WebSocketsServer webSocket;

// Funciones de WebServer y WiFi
void initWiFiAP();
void initWebServer();
void handleRoot();
void handleNotFound();
void handleGetMessages();
void handleSendMessage();
void broadcastBLEMessage();
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);

#endif
