#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include <Arduino.h>

// Tipos de funciones que pueden ser tareas
typedef void (*TaskFunction)(void);

// Estructura para definir una tarea
struct Task {
  const char* name;
  TaskFunction function;
  unsigned long interval;       // Intervalo en milisegundos
  unsigned long lastExecution;  // Timestamp de última ejecución
  bool enabled;
  unsigned long executionCount; // Contador de ejecuciones
};

class TaskScheduler {
private:
  static const int MAX_TASKS = 16;
  Task tasks[MAX_TASKS];
  int taskCount;

public:
  TaskScheduler();
  
  /**
   * @brief Añade una nueva tarea al scheduler
   * @param name Nombre de la tarea (para debugging)
   * @param function Función a ejecutar
   * @param interval Intervalo en ms (0 = cada loop)
   * @return true si se añadió exitosamente, false si está lleno
   */
  bool addTask(const char* name, TaskFunction function, unsigned long interval = 0);
  
  /**
   * @brief Elimina una tarea por nombre
   * @param name Nombre de la tarea
   * @return true si se eliminó, false si no existe
   */
  bool removeTask(const char* name);
  
  /**
   * @brief Actualiza el scheduler - debe llamarse en cada loop
   * Ejecuta las tareas cuyo intervalo ha vencido
   */
  void update();
  
  /**
   * @brief Habilita una tarea
   * @param name Nombre de la tarea
   */
  void enableTask(const char* name);
  
  /**
   * @brief Deshabilita una tarea sin eliminarla
   * @param name Nombre de la tarea
   */
  void disableTask(const char* name);
  
  /**
   * @brief Obtiene información de una tarea
   * @param name Nombre de la tarea
   * @return Puntero a la tarea, nullptr si no existe
   */
  Task* getTask(const char* name);
  
  /**
   * @brief Imprime información de todas las tareas
   */
  void printTasks();
  
  /**
   * @brief Obtiene el número de tareas registradas
   */
  int getTaskCount() const { return taskCount; }
};

#endif
