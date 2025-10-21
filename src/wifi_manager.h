#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include "config.h"

namespace WiFiManager {
  // Inicjalizacja WiFi
  void init();
  
  // Reset WiFi
  void reset();
  
  // Połączenie z siecią
  bool connect(const String& ssid, const String& password);
  
  // Zarządzanie danymi
  void saveCredentials(const String& ssid, const String& password);
  bool loadCredentials(String& ssid, String& password);
  
  // Status
  String getIP();
  bool isConnected();
  bool isAPMode();
}

#endif // WIFI_MANAGER_H
