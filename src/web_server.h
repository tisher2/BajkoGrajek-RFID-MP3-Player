#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include "config.h"

namespace WebServer {
  // Inicjalizacja serwera Web
  void init();
  
  // Pomocnicze funkcje
  String getStatusJSON();
  String getFilesJSON(const String& path);
}

#endif // WEB_SERVER_H
