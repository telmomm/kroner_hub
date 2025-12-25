#ifndef WEBSERVER_FUNCTIONS_H
#define WEBSERVER_FUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>

// Declaración de variables globales WebServer
extern WebServer webServer;
extern DNSServer dnsServer;

// Funciones de WebServer y WiFi
void initWiFiAP();
void initWebServer();
void handleRoot();
void handleNotFound();
void handleGetMessages();
void handleSendMessage();

#endif
