#include <Arduino.h>
#include "kroner_config.h"
#include "task_functions.h"
#include "ble_functions.h"
#include "webserver_functions.h"
#include "input_functions.h"
#include "serial_functions.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void setup() {
  Serial.begin(SERIAL_BAUD_USB);
  delay(500);  // Esperar a que Serial esté listo
  
  // Mostrar información de firmware al inicio
  printBootBanner();

  // Inicializar módulos
  DEBUG_PRINTLN("Initializing modules...");
  initWiFiAP();
  initWebServer();
  initInputs();
  initAPC220();
  initBLE();

  DEBUG_PRINTLN("\nStarting FreeRTOS tasks (pinned cores)...");
  startSystemTasks();
  DEBUG_PRINTLN("Setup complete!\n");
}

void loop() {
  // Loop principal queda libre; las tareas corren en hilos FreeRTOS
  vTaskDelay(pdMS_TO_TICKS(1000));
}
