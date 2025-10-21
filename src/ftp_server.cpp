#include "ftp_server.h"
#include <SimpleFTPServer.h>
#include <SD.h>

namespace FTPServer {
  // Serwer FTP
  static FtpServer* ftpServer = nullptr;
  
  // ========================================================================
  // INICJALIZACJA
  // ========================================================================
  
  void init() {
    Serial.println("[FTP] Inicjalizacja serwera FTP...");
    
    // Utworzenie serwera FTP
    ftpServer = new FtpServer();
    
    // Konfiguracja
    ftpServer->begin(FTP_USER, FTP_PASS);
    
    Serial.println("[FTP] Serwer FTP uruchomiony");
    Serial.printf("[FTP] User: %s\n", FTP_USER);
    Serial.printf("[FTP] Pass: %s\n", FTP_PASS);
    Serial.printf("[FTP] Port: %d\n", FTP_PORT);
    
    addDisplayLog("FTP OK");
  }
  
  // ========================================================================
  // OBSŁUGA
  // ========================================================================
  
  void handle() {
    if (ftpServer) {
      ftpServer->handleFTP();
    }
  }
}
