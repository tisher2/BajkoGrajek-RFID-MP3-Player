#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Arduino.h>
#include "config.h"

namespace DisplayHandler {
  // Inicjalizacja modułu wyświetlacza
  void init();
  
  // Utworzenie FreeRTOS task
  void createTask();
  
  // Task wyświetlacza (FreeRTOS)
  void displayTask(void* parameter);
  
  // Aktualizacja wyświetlacza
  void update();
  void clear();
  
  // Wyświetlanie
  void showStatus();
  void showLog(const String& msg);
}

#endif // DISPLAY_HANDLER_H
