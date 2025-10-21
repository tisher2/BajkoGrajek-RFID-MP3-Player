#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
#include "config.h"

namespace ButtonHandler {
  // Inicjalizacja modułu przycisków
  void init();
  
  // Utworzenie FreeRTOS task
  void createTask();
  
  // Task przycisków (FreeRTOS)
  void buttonTask(void* parameter);
  
  // Sprawdzenie resetu WiFi
  bool isResetWiFiPressed();
}

#endif // BUTTON_HANDLER_H
