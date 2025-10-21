#include "config_manager.h"
#include <SD.h>
#include <FS.h>

namespace ConfigManager {
  // Lista kart RFID
  static std::vector<RFIDCard> rfidCards;
  
  // Ścieżka do pliku konfiguracyjnego
  static const char* CONFIG_FILE = "/config.csv";
  
  // ========================================================================
  // ŁADOWANIE I ZAPISYWANIE
  // ========================================================================
  
  void load() {
    Serial.println("[Config] Ładowanie konfiguracji...");
    
    rfidCards.clear();
    
    if (!SD.exists(CONFIG_FILE)) {
      Serial.println("[Config] Plik konfiguracyjny nie istnieje - tworzę pusty");
      
      File file = SD.open(CONFIG_FILE, FILE_WRITE);
      if (file) {
        file.println("# BajkoGrajek - Konfiguracja kart RFID");
        file.println("# Format: UID;typ;ścieżka_lub_akcja");
        file.println("# Typ: file, dir, ctrl");
        file.println("# Przykłady:");
        file.println("# 04:A1:B2:C3;file;/muzyka/piosenka.mp3");
        file.println("# 04:D4:E5:F6;dir;/muzyka/album");
        file.println("# 04:11:22:33;ctrl;REPEAT3");
        file.println("# 04:44:55:66;ctrl;SHUFFLE_ON");
        file.println("# 04:77:88:99;ctrl;SLEEP_TIMER:30");
        file.close();
        Serial.println("[Config] Utworzono pusty plik konfiguracyjny");
      } else {
        Serial.println("[Config] BŁĄD: Nie można utworzyć pliku konfiguracyjnego!");
        addDisplayLog("Config BŁĄD!");
        return;
      }
      
      addDisplayLog("Config OK");
      return;
    }
    
    File file = SD.open(CONFIG_FILE, FILE_READ);
    if (!file) {
      Serial.println("[Config] BŁĄD: Nie można otworzyć pliku konfiguracyjnego!");
      addDisplayLog("Config BŁĄD!");
      return;
    }
    
    int lineCount = 0;
    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();
      
      // Pomiń puste linie i komentarze
      if (line.length() == 0 || line.startsWith("#")) {
        continue;
      }
      
      // Parsuj linię: UID;type;path_or_action
      int firstSemi = line.indexOf(';');
      int secondSemi = line.indexOf(';', firstSemi + 1);
      
      if (firstSemi == -1 || secondSemi == -1) {
        Serial.printf("[Config] UWAGA: Nieprawidłowa linia: %s\n", line.c_str());
        continue;
      }
      
      RFIDCard card;
      card.uid = line.substring(0, firstSemi);
      String typeStr = line.substring(firstSemi + 1, secondSemi);
      String pathOrAction = line.substring(secondSemi + 1);
      
      card.uid.toUpperCase();
      
      // Określ typ karty
      if (typeStr == "file") {
        card.type = CARD_FILE;
        card.path = pathOrAction;
      } else if (typeStr == "dir") {
        card.type = CARD_DIR;
        card.path = pathOrAction;
      } else if (typeStr == "ctrl") {
        card.type = CARD_CTRL;
        
        // Parsuj akcję sterującą
        if (pathOrAction == "REPEAT3") {
          card.action = CTRL_REPEAT_3;
        } else if (pathOrAction == "REPEAT5") {
          card.action = CTRL_REPEAT_5;
        } else if (pathOrAction == "REPEAT_INF") {
          card.action = CTRL_REPEAT_INF;
        } else if (pathOrAction == "SHUFFLE_ON") {
          card.action = CTRL_SHUFFLE_ON;
        } else if (pathOrAction.startsWith("SLEEP_TIMER:")) {
          card.action = CTRL_SLEEP_TIMER;
          int colonIdx = pathOrAction.indexOf(':');
          if (colonIdx != -1) {
            card.value = pathOrAction.substring(colonIdx + 1).toInt();
          }
        } else {
          Serial.printf("[Config] UWAGA: Nieznana akcja sterująca: %s\n", pathOrAction.c_str());
          continue;
        }
      } else {
        Serial.printf("[Config] UWAGA: Nieznany typ karty: %s\n", typeStr.c_str());
        continue;
      }
      
      rfidCards.push_back(card);
      lineCount++;
    }
    
    file.close();
    
    Serial.printf("[Config] Załadowano %d kart\n", lineCount);
    addDisplayLog("Config: " + String(lineCount) + " kart");
  }
  
  void save() {
    Serial.println("[Config] Zapisywanie konfiguracji...");
    
    // Atomic write - najpierw zapisz do pliku tymczasowego
    const char* TEMP_FILE = "/config.csv.tmp";
    
    File file = SD.open(TEMP_FILE, FILE_WRITE);
    if (!file) {
      Serial.println("[Config] BŁĄD: Nie można utworzyć pliku tymczasowego!");
      return;
    }
    
    // Nagłówek
    file.println("# BajkoGrajek - Konfiguracja kart RFID");
    file.println("# Format: UID;typ;ścieżka_lub_akcja");
    file.println("# Typ: file, dir, ctrl");
    
    // Karty
    for (const RFIDCard& card : rfidCards) {
      file.print(card.uid);
      file.print(";");
      
      switch (card.type) {
        case CARD_FILE:
          file.print("file;");
          file.println(card.path);
          break;
        case CARD_DIR:
          file.print("dir;");
          file.println(card.path);
          break;
        case CARD_CTRL:
          file.print("ctrl;");
          file.println(ctrlActionToString(card.action, card.value));
          break;
      }
    }
    
    file.close();
    
    // Usuń stary plik i zmień nazwę
    SD.remove(CONFIG_FILE);
    SD.rename(TEMP_FILE, CONFIG_FILE);
    
    Serial.printf("[Config] Zapisano %d kart\n", rfidCards.size());
  }
  
  // ========================================================================
  // ZARZĄDZANIE KARTAMI
  // ========================================================================
  
  void addCard(const RFIDCard& card) {
    // Sprawdź czy karta już istnieje
    for (size_t i = 0; i < rfidCards.size(); i++) {
      if (rfidCards[i].uid == card.uid) {
        // Zaktualizuj istniejącą kartę
        rfidCards[i] = card;
        Serial.printf("[Config] Zaktualizowano kartę: %s\n", card.uid.c_str());
        save();
        return;
      }
    }
    
    // Dodaj nową kartę
    rfidCards.push_back(card);
    Serial.printf("[Config] Dodano kartę: %s\n", card.uid.c_str());
    save();
  }
  
  void removeCard(const String& uid) {
    String uidUpper = uid;
    uidUpper.toUpperCase();
    
    for (auto it = rfidCards.begin(); it != rfidCards.end(); ++it) {
      if (it->uid == uidUpper) {
        rfidCards.erase(it);
        Serial.printf("[Config] Usunięto kartę: %s\n", uidUpper.c_str());
        save();
        return;
      }
    }
    
    Serial.printf("[Config] UWAGA: Nie znaleziono karty do usunięcia: %s\n", uidUpper.c_str());
  }
  
  RFIDCard findCard(const String& uid) {
    String uidUpper = uid;
    uidUpper.toUpperCase();
    
    for (const RFIDCard& card : rfidCards) {
      if (card.uid == uidUpper) {
        return card;
      }
    }
    
    // Zwróć pustą kartę jeśli nie znaleziono
    return RFIDCard();
  }
  
  std::vector<RFIDCard> getCards() {
    return rfidCards;
  }
  
  int getCardCount() {
    return rfidCards.size();
  }
}
