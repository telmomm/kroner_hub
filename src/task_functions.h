#ifndef TASK_FUNCTIONS_H
#define TASK_FUNCTIONS_H

#include <Arduino.h>

// Funciones de tarea (una iteración) reutilizadas por los hilos FreeRTOS
// Todas son no-bloqueantes
void taskHandleWebServer();
void taskHandleBLE();
void taskScanInputs();
void taskProcessRadio();
void taskDebugStatus();

// Inicializa y arranca las tareas FreeRTOS fijadas a cada núcleo
void startSystemTasks();

#endif
