#include "kroner_config.h"
#include "webserver_functions.h"
#include "ble_functions.h"

// Instancias globales
WebServer webServer(80);
DNSServer dnsServer;

void initWiFiAP() {
  DEBUG_PRINTLN("=================================");
  DEBUG_PRINTLN("Iniciando WiFi AP...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);  // SSID sin contraseña por defecto
  IPAddress apIP(WIFI_AP_IP0, WIFI_AP_IP1, WIFI_AP_IP2, WIFI_AP_IP3);
  WiFi.softAPConfig(apIP, apIP, IPAddress(WIFI_AP_MASK0, WIFI_AP_MASK1, WIFI_AP_MASK2, WIFI_AP_MASK3));
  DEBUG_PRINT("AP IP: ");
  DEBUG_PRINTLN(WiFi.softAPIP());

  // Configurar DNS para Captive Portal
  dnsServer.start(53, "*", apIP);
  DEBUG_PRINTLN("DNS Server iniciado para Captive Portal");
}

void initWebServer() {
  // Inicializar LittleFS
  if (!LittleFS.begin()) {
    DEBUG_PRINTLN("Error al inicializar LittleFS");
  } else {
    DEBUG_PRINTLN("LittleFS iniciado correctamente");
  }

  // Configurar rutas
  webServer.on("/", handleRoot);
  webServer.on("/api/messages", handleGetMessages);
  webServer.on("/api/send", HTTP_POST, handleSendMessage);
  webServer.onNotFound(handleNotFound);
  webServer.begin();
  DEBUG_PRINTLN("Web Server iniciado en puerto 80");
  DEBUG_PRINTLN("=================================");
}

void handleRoot() {
  if (LittleFS.exists("/index.html")) {
    File file = LittleFS.open("/index.html", "r");
    webServer.streamFile(file, "text/html");
    file.close();
  } else {
    webServer.send(404, "text/plain", "index.html no encontrado");
  }
}

void handleNotFound() {
  String path = webServer.uri();
  if (!path.startsWith("/api/")) {
    webServer.sendHeader("Location", "http://192.168.4.1/", true);
    webServer.send(302, "text/plain", "");
  } else {
    webServer.send(404, "text/plain", "Not found");
  }
}

void handleGetMessages() {
  char jsonResponse[512];
  
  // Convertir buffer a base64
  char base64Buffer[400];
  int base64Len = 0;
  if (lastMessageLen > 0) {
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
