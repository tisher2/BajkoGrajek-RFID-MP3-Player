#ifndef FTP_SERVER_H
#define FTP_SERVER_H

#include <Arduino.h>
#include "config.h"

namespace FTPServer {
  // Inicjalizacja serwera FTP
  void init();
  
  // Obsługa serwera (wywołuj w loop)
  void handle();
}

#endif // FTP_SERVER_H
