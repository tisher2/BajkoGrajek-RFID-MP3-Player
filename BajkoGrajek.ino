/*
 * BajkoGrajek - RFID MP3 Player
 * 
 * Odtwarzacz MP3 sterowany kartami RFID z interfejsem WWW
 * 
 * Funkcje:
 * - Odtwarzanie MP3 z karty SD (I2S)
 * - Skanowanie kart RFID (PN532)
 * - Fizyczna playlista (do 10 kart jednocześnie)
 * - Karty sterujące (REPEAT, SHUFFLE, SLEEP_TIMER)
 * - Wyświetlacz OLED (status, logi)
 * - 4 przyciski sterujące (VOL-, PAUSE, PLAY, VOL+)
 * - Serwer WWW (konfiguracja, przeglądarka plików)
 * - Serwer FTP (zarządzanie plikami)
 * - OTA Update
 * - WiFi AP/STA
 * 
 * Hardware:
 * - ESP32
 * - Karta SD (SPI)
 * - I2S DAC
 * - PN532 RFID (I2C)
 * - OLED SSD1306 (I2C)
 * - 4 przyciski
 * 
 * Autor: BajkoGrajek Team
 * Licencja: MIT
 */

#include "src/config.h"
#include "src/audio_handler.h"
#include "src/rfid_handler.h"
#include "src/display_handler.h"
#include "src/button_handler.h"
#include "src/web_server.h"
#include "src/ftp_server.h"
#include "src/config_manager.h"
#include "src/wifi_manager.h"

void setup() {
  // Inicjalizacja Serial
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n");
  Serial.println("===========================================");
  Serial.println("       BajkoGrajek - RFID MP3 Player      ");
  Serial.println("===========================================");
  Serial.println();
  
  // Inicjalizacja przycisków (musi być pierwsza)
  ButtonHandler::init();
  
  // Sprawdź reset WiFi (przyciski 1+4 przy starcie)
  if (ButtonHandler::isResetWiFiPressed()) {
    Serial.println("[Main] Wykryto kombinację resetu WiFi!");
    WiFiManager::reset();
    // Po resecie nastąpi restart, więc kod poniżej się nie wykona
  }
  
  // Inicjalizacja wyświetlacza
  DisplayHandler::init();
  
  // Inicjalizacja audio
  AudioHandler::init();
  
  // Inicjalizacja RFID
  RFIDHandler::init();
  
  // Wczytaj konfigurację kart
  ConfigManager::load();
  
  // Inicjalizacja WiFi
  WiFiManager::init();
  
  // Inicjalizacja serwerów
  WebServer::init();
  FTPServer::init();
  
  // Utworzenie FreeRTOS tasks
  Serial.println();
  Serial.println("[Main] Tworzenie FreeRTOS tasks...");
  AudioHandler::createTask();
  RFIDHandler::createTask();
  ButtonHandler::createTask();
  DisplayHandler::createTask();
  
  Serial.println();
  Serial.println("[Main] Inicjalizacja zakończona!");
  Serial.println("===========================================");
  Serial.println();
  
  addDisplayLog("Gotowy!");
}

void loop() {
  // Obsługa serwera FTP
  FTPServer::handle();
  
  // Krótkie opóźnienie
  vTaskDelay(pdMS_TO_TICKS(100));
}
