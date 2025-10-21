#include "wifi_manager.h"
#include <WiFi.h>
#include <Preferences.h>

namespace WiFiManager {
  // Preferences
  static Preferences prefs;
  
  // Tryb
  static bool apMode = false;
  
  // ========================================================================
  // INICJALIZACJA
  // ========================================================================
  
  void init() {
    Serial.println("[WiFi] Inicjalizacja modułu WiFi...");
    
    // Spróbuj wczytać dane z pamięci
    String ssid, password;
    if (loadCredentials(ssid, password) && ssid.length() > 0) {
      Serial.println("[WiFi] Znaleziono zapisane dane sieci");
      Serial.printf("[WiFi] Łączenie z: %s\n", ssid.c_str());
      
      // Tryb STA
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid.c_str(), password.c_str());
      
      // Czekaj na połączenie (max 10 sekund)
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
      }
      Serial.println();
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WiFi] Połączono!");
        Serial.print("[WiFi] IP: ");
        Serial.println(WiFi.localIP());
        addDisplayLog("WiFi: " + WiFi.localIP().toString());
        apMode = false;
        return;
      } else {
        Serial.println("[WiFi] Nie udało się połączyć");
      }
    }
    
    // Tryb AP
    Serial.println("[WiFi] Uruchamiam tryb Access Point");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    Serial.print("[WiFi] AP SSID: ");
    Serial.println(AP_SSID);
    Serial.print("[WiFi] AP IP: ");
    Serial.println(WiFi.softAPIP());
    
    addDisplayLog("AP: " + String(AP_SSID));
    apMode = true;
  }
  
  // ========================================================================
  // RESET
  // ========================================================================
  
  void reset() {
    Serial.println("[WiFi] Reset WiFi - usuwam dane...");
    
    // Usuń dane z pamięci
    prefs.begin("wifi", false);
    prefs.clear();
    prefs.end();
    
    // Sygnał dźwiękowy 5kHz przez 1s
    #ifdef BUZZER_PIN
    tone(BUZZER_PIN, 5000);
    delay(1000);
    noTone(BUZZER_PIN);
    #endif
    
    Serial.println("[WiFi] Dane WiFi usunięte. Restart...");
    delay(1000);
    ESP.restart();
  }
  
  // ========================================================================
  // POŁĄCZENIE
  // ========================================================================
  
  bool connect(const String& ssid, const String& password) {
    Serial.printf("[WiFi] Łączenie z: %s\n", ssid.c_str());
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Czekaj na połączenie (max 10 sekund)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("[WiFi] Połączono!");
      Serial.print("[WiFi] IP: ");
      Serial.println(WiFi.localIP());
      apMode = false;
      return true;
    } else {
      Serial.println("[WiFi] Nie udało się połączyć");
      return false;
    }
  }
  
  // ========================================================================
  // ZARZĄDZANIE DANYMI
  // ========================================================================
  
  void saveCredentials(const String& ssid, const String& password) {
    Serial.println("[WiFi] Zapisywanie danych sieci...");
    
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("password", password);
    prefs.end();
    
    Serial.println("[WiFi] Dane zapisane");
  }
  
  bool loadCredentials(String& ssid, String& password) {
    prefs.begin("wifi", true);
    ssid = prefs.getString("ssid", "");
    password = prefs.getString("password", "");
    prefs.end();
    
    return ssid.length() > 0;
  }
  
  // ========================================================================
  // STATUS
  // ========================================================================
  
  String getIP() {
    if (apMode) {
      return WiFi.softAPIP().toString();
    } else {
      return WiFi.localIP().toString();
    }
  }
  
  bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
  }
  
  bool isAPMode() {
    return apMode;
  }
}
