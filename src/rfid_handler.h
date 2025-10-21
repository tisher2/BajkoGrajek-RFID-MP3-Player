#ifndef RFID_HANDLER_H
#define RFID_HANDLER_H

#include <Arduino.h>
#include "config.h"

namespace RFIDHandler {
  // Inicjalizacja modułu RFID
  void init();
  
  // Utworzenie FreeRTOS task
  void createTask();
  
  // Task RFID (FreeRTOS)
  void rfidTask(void* parameter);
  
  // Skanowanie
  String scanCard();
  String getLastScanned();
  void startWebScan();
  void stopWebScan();
  bool isWebScanning();
  
  // Kolejka odtwarzania
  void addToQueue(const RFIDCard& card);
  void removeFromQueue(const String& uid);
  bool isInQueue(const String& uid);
  void clearQueue();
  int getQueueSize();
}

#endif // RFID_HANDLER_H
