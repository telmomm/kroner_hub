#include "task_scheduler.h"

TaskScheduler::TaskScheduler() : taskCount(0) {
  for (int i = 0; i < MAX_TASKS; i++) {
    tasks[i].function = nullptr;
    tasks[i].enabled = false;
  }
}

bool TaskScheduler::addTask(const char* name, TaskFunction function, unsigned long interval) {
  if (taskCount >= MAX_TASKS) {
    Serial.printf("[TaskScheduler] ERROR: Max tasks (%d) reached!\n", MAX_TASKS);
    return false;
  }

  // Verificar que no exista una tarea con el mismo nombre
  for (int i = 0; i < taskCount; i++) {
    if (strcmp(tasks[i].name, name) == 0) {
      Serial.printf("[TaskScheduler] ERROR: Task '%s' already exists!\n", name);
      return false;
    }
  }

  tasks[taskCount].name = name;
  tasks[taskCount].function = function;
  tasks[taskCount].interval = interval;
  tasks[taskCount].lastExecution = 0;
  tasks[taskCount].enabled = true;
  tasks[taskCount].executionCount = 0;

  Serial.printf("[TaskScheduler] Task '%s' added (interval: %lu ms)\n", name, interval);
  taskCount++;

  return true;
}

bool TaskScheduler::removeTask(const char* name) {
  for (int i = 0; i < taskCount; i++) {
    if (strcmp(tasks[i].name, name) == 0) {
      // Desplazar tareas posteriores
      for (int j = i; j < taskCount - 1; j++) {
        tasks[j] = tasks[j + 1];
      }
      taskCount--;
      Serial.printf("[TaskScheduler] Task '%s' removed\n", name);
      return true;
    }
  }
  Serial.printf("[TaskScheduler] ERROR: Task '%s' not found!\n", name);
  return false;
}

void TaskScheduler::update() {
  unsigned long currentTime = millis();

  for (int i = 0; i < taskCount; i++) {
    if (!tasks[i].enabled || tasks[i].function == nullptr) {
      continue;
    }

    // Verificar si es tiempo de ejecutar
    if (tasks[i].interval == 0 || (currentTime - tasks[i].lastExecution) >= tasks[i].interval) {
      // Ejecutar tarea
      tasks[i].function();
      tasks[i].lastExecution = currentTime;
      tasks[i].executionCount++;
    }
  }
}

void TaskScheduler::enableTask(const char* name) {
  for (int i = 0; i < taskCount; i++) {
    if (strcmp(tasks[i].name, name) == 0) {
      tasks[i].enabled = true;
      Serial.printf("[TaskScheduler] Task '%s' enabled\n", name);
      return;
    }
  }
  Serial.printf("[TaskScheduler] ERROR: Task '%s' not found!\n", name);
}

void TaskScheduler::disableTask(const char* name) {
  for (int i = 0; i < taskCount; i++) {
    if (strcmp(tasks[i].name, name) == 0) {
      tasks[i].enabled = false;
      Serial.printf("[TaskScheduler] Task '%s' disabled\n", name);
      return;
    }
  }
  Serial.printf("[TaskScheduler] ERROR: Task '%s' not found!\n", name);
}

Task* TaskScheduler::getTask(const char* name) {
  for (int i = 0; i < taskCount; i++) {
    if (strcmp(tasks[i].name, name) == 0) {
      return &tasks[i];
    }
  }
  return nullptr;
}

void TaskScheduler::printTasks() {
  Serial.println("\n=== Task Scheduler Status ===");
  Serial.printf("Total tasks: %d/%d\n", taskCount, MAX_TASKS);
  
  for (int i = 0; i < taskCount; i++) {
    Serial.printf("[%d] %s | Interval: %lu ms | Enabled: %s | Executions: %lu\n",
                  i,
                  tasks[i].name,
                  tasks[i].interval,
                  tasks[i].enabled ? "YES" : "NO",
                  tasks[i].executionCount);
  }
  Serial.println("==============================\n");
}
