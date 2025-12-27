#include "task_functions.h"
#include "kroner_config.h"
#include "ble_functions.h"
#include "webserver_functions.h"
#include "input_functions.h"
#include "serial_functions.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Variables de estado para el BLE (compartidas entre núcleos)
static volatile bool bleConnected = false;
static volatile unsigned long bleConnectionTime = 0;

// Intervalos de tareas (ticks)
static constexpr TickType_t WEB_SERVER_DELAY = pdMS_TO_TICKS(50);   // 20 Hz
static constexpr TickType_t BLE_DELAY = pdMS_TO_TICKS(20);          // 50 Hz
static constexpr TickType_t INPUT_DELAY = pdMS_TO_TICKS(10);        // 100 Hz para keypad
static constexpr TickType_t RADIO_DELAY = pdMS_TO_TICKS(200);       // 5 Hz
static constexpr TickType_t DEBUG_DELAY = pdMS_TO_TICKS(5000);      // 0.2 Hz

// Handles de tareas FreeRTOS
static TaskHandle_t webServerTaskHandle = nullptr;
static TaskHandle_t bleTaskHandle = nullptr;
static TaskHandle_t inputTaskHandle = nullptr;
static TaskHandle_t radioTaskHandle = nullptr;
static TaskHandle_t debugTaskHandle = nullptr;

// Declaraciones de tareas FreeRTOS
static void webServerTask(void* pvParameters);
static void bleTask(void* pvParameters);
static void inputTask(void* pvParameters);
static void radioTask(void* pvParameters);
static void debugTask(void* pvParameters);

void startSystemTasks() {
  // Núcleo 0: WiFi/Red y radio para convivir con tareas del stack WiFi
  xTaskCreatePinnedToCore(webServerTask, "WebServer", 4096, nullptr, 2, &webServerTaskHandle, 0);
  xTaskCreatePinnedToCore(radioTask, "Radio", 4096, nullptr, 2, &radioTaskHandle, 0);

  // Núcleo 1: BLE, entradas y debug ligero
  xTaskCreatePinnedToCore(bleTask, "BLE", 4096, nullptr, 3, &bleTaskHandle, 1);
  xTaskCreatePinnedToCore(inputTask, "Inputs", 4096, nullptr, 3, &inputTaskHandle, 1);
  xTaskCreatePinnedToCore(debugTask, "Debug", 3072, nullptr, 1, &debugTaskHandle, 1);
}

/**
 * @brief Tarea: Maneja las peticiones HTTP y DNS
 * Intervalo: 50ms
 */
void taskHandleWebServer() {
  webServer.handleClient();
  dnsServer.processNextRequest();
}

/**
 * @brief Tarea: Maneja la conexión BLE
 * Intervalo: 100ms
 */
void taskHandleBLE() {
  // Mantener la pila BLE viva
  BLE.poll();

  BLEDevice central = BLE.central();
  
  if (central) {
    // Hay un cliente BLE conectado
    if (!bleConnected) {
      // Acaba de conectarse
      bleConnected = true;
      bleConnectionTime = millis();
      
      DEBUG_PRINT("BLE Connected: ");
      DEBUG_PRINTLN(central.address());
      
      // Enviar información de firmware
      sendFirmwareInfo();
      delay(200);
      
      // Enviar estado inicial de switches
      DEBUG_PRINTLN("Sending initial switch states...");
      sendInitialSwitchState(0, INPUT7PIN);
      sendInitialSwitchState(1, INPUT8PIN);
      sendInitialSwitchState(2, INPUT9PIN);
    }
  } else {
    // No hay cliente BLE
    if (bleConnected) {
      DEBUG_PRINTLN("BLE Disconnected");
      bleConnected = false;
    }
  }
}

/**
 * @brief Tarea: Escanea entradas (keypad, switches, interrupts)
 * Intervalo: ~10ms
 */
void taskScanInputs() {
  // Keypad siempre se escanea
  scanKeypad();

  // Escanear switches solo si hay conexión BLE
  if (bleConnected) {
    scanSwitch(0, INPUT7PIN);
    scanSwitch(1, INPUT8PIN);
    scanSwitch(2, INPUT9PIN);

    // Enviar valores de F1, F2, F3 si hay cambios
    if (newInputValue) {
      uint32_t message[4] = {F1, F2, F3, 0};
      
      DEBUG_PRINT("Inputs: F1=");
      DEBUG_PRINT(F1);
      DEBUG_PRINT(" F2=");
      DEBUG_PRINT(F2);
      DEBUG_PRINT(" F3=");
      DEBUG_PRINTLN(F3);

      pulsadorCharacteristic.writeValue((uint8_t*)message, sizeof(message));
      newInputValue = false;
    }
  }
}

/**
 * @brief Tarea: Procesa datos del módulo APC220
 * Intervalo: 200ms
 * 
 * Envía datos que llegaron por BLE al APC220
 */
void taskProcessRadio() {
  // Si hay un nuevo mensaje BLE listo
  if (bleMessageReady) {
    bleMessageReady = false;
    
    // Reenviar al APC220 por Serial2
    // Necesitamos hacer una copia temporal para evitar problemas con volatile
    uint8_t tempBuffer[255];
    int tempLen = bleMessageLen;
    memcpy(tempBuffer, (const void*)bleMessageBuffer, tempLen);
    
    Serial2.write(tempBuffer, tempLen);
    Serial2.flush();
    
    DEBUG_PRINT("BLE->APC220 (bytes): ");
    DEBUG_PRINTLN(tempLen);
  }
}

/**
 * @brief Tarea: Imprime estado del sistema para debugging
 * Intervalo: 5000ms (5 segundos)
 */
void taskDebugStatus() {
  DEBUG_PRINTLN("\n=== System Status ===");
  DEBUG_PRINT("Free Heap: ");
  DEBUG_PRINT(ESP.getFreeHeap());
  DEBUG_PRINTLN(" bytes");
  
  DEBUG_PRINT("BLE Connected: ");
  if (bleConnected) {
    unsigned long connTime = (millis() - bleConnectionTime) / 1000;
    DEBUG_PRINT("YES (");
    DEBUG_PRINT(connTime);
    DEBUG_PRINTLN("s)");
  } else {
    DEBUG_PRINTLN("NO");
  }
  
  DEBUG_PRINT("WiFi SSID: ");
  DEBUG_PRINTLN(WIFI_AP_SSID);
  DEBUG_PRINTLN("===================\n");
}

// =============================
// Tareas FreeRTOS fijadas a núcleos
// =============================
static void webServerTask(void* pvParameters) {
  (void)pvParameters;
  for (;;) {
    taskHandleWebServer();
    vTaskDelay(WEB_SERVER_DELAY);
  }
}

static void bleTask(void* pvParameters) {
  (void)pvParameters;
  for (;;) {
    taskHandleBLE();
    vTaskDelay(BLE_DELAY);
  }
}

static void inputTask(void* pvParameters) {
  (void)pvParameters;
  for (;;) {
    taskScanInputs();
    vTaskDelay(INPUT_DELAY);
  }
}

static void radioTask(void* pvParameters) {
  (void)pvParameters;
  for (;;) {
    taskProcessRadio();
    vTaskDelay(RADIO_DELAY);
  }
}

static void debugTask(void* pvParameters) {
  (void)pvParameters;
  for (;;) {
    taskDebugStatus();
    vTaskDelay(DEBUG_DELAY);
  }
}

