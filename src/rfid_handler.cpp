#include "rfid_handler.h"
#include "config_manager.h"
#include "audio_handler.h"
#include <Wire.h>
#include <Adafruit_PN532.h>

namespace RFIDHandler {
  // Obiekt PN532
  static Adafruit_PN532* nfc = nullptr;
  
  // Handle task
  static TaskHandle_t rfidTaskHandle = nullptr;
  
  // Anti-debounce
  static String lastUID = "";
  static unsigned long lastScanTime = 0;
  
  // Karty w czytniku (do wykrywania usunięcia)
  static std::vector<String> cardsInReader;
  
  // ========================================================================
  // INICJALIZACJA
  // ========================================================================
  
  void init() {
    Serial.println("[RFID] Inicjalizacja modułu RFID...");
    
    // Inicjalizacja I2C1 dla PN532
    Wire1.begin(I2C1_SDA, I2C1_SCL);
    Wire1.setClock(I2C_FREQUENCY);
    
    // Inicjalizacja PN532
    nfc = new Adafruit_PN532(I2C1_SDA, I2C1_SCL, &Wire1);
    nfc->begin();
    
    uint32_t versiondata = nfc->getFirmwareVersion();
    if (!versiondata) {
      Serial.println("[RFID] BŁĄD: Nie znaleziono modułu PN532!");
      addDisplayLog("RFID BŁĄD!");
      return;
    }
    
    Serial.print("[RFID] Znaleziono PN532 v");
    Serial.println((versiondata >> 24) & 0xFF, HEX);
    addDisplayLog("RFID OK");
    
    // Konfiguracja do odczytu pasywnego
    nfc->SAMConfig();
    
    // Inicjalizacja mutexa kolejki
    if (queueMutex == nullptr) {
      queueMutex = xSemaphoreCreateMutex();
    }
    
    // Inicjalizacja mutexa skanowania
    if (scanMutex == nullptr) {
      scanMutex = xSemaphoreCreateMutex();
    }
  }
  
  // ========================================================================
  // FREERTOS TASK
  // ========================================================================
  
  void createTask() {
    xTaskCreatePinnedToCore(
      rfidTask,
      "RFIDTask",
      4096,
      nullptr,
      5,  // Średni priorytet
      &rfidTaskHandle,
      0   // Core 0
    );
    Serial.println("[RFID] Task utworzony na Core 0");
  }
  
  void rfidTask(void* parameter) {
    while (true) {
      // Skanuj kartę
      String uid = scanCard();
      
      if (uid.length() > 0) {
        // Anti-debounce
        if (uid == lastUID && (millis() - lastScanTime) < RFID_DEBOUNCE_MS) {
          vTaskDelay(pdMS_TO_TICKS(RFID_SCAN_INTERVAL_MS));
          continue;
        }
        
        lastUID = uid;
        lastScanTime = millis();
        
        // Tryb skanowania dla Web UI
        if (webScanMode) {
          if (xSemaphoreTake(scanMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            lastScannedUID = uid;
            webScanMode = false;
            xSemaphoreGive(scanMutex);
          }
          Serial.printf("[RFID] Zeskanowano dla Web UI: %s\n", uid.c_str());
          vTaskDelay(pdMS_TO_TICKS(RFID_SCAN_INTERVAL_MS));
          continue;
        }
        
        // Sprawdź czy karta już jest w czytniku
        bool alreadyInReader = false;
        for (const String& existingUID : cardsInReader) {
          if (existingUID == uid) {
            alreadyInReader = true;
            break;
          }
        }
        
        if (!alreadyInReader) {
          // Nowa karta
          Serial.printf("[RFID] Wykryto nową kartę: %s\n", uid.c_str());
          cardsInReader.push_back(uid);
          
          // Znajdź kartę w konfiguracji
          RFIDCard card = ConfigManager::findCard(uid);
          if (card.uid.length() > 0) {
            Serial.printf("[RFID] Znaleziono konfigurację dla karty: %s\n", uid.c_str());
            
            // Obsługa karty sterującej
            if (card.type == CARD_CTRL) {
              Serial.println("[RFID] Karta sterująca");
              
              switch (card.action) {
                case CTRL_REPEAT_3:
                  AudioHandler::setRepeat(3);
                  break;
                case CTRL_REPEAT_5:
                  AudioHandler::setRepeat(5);
                  break;
                case CTRL_REPEAT_INF:
                  AudioHandler::setRepeat(0);
                  break;
                case CTRL_SHUFFLE_ON:
                  AudioHandler::setShuffle(true);
                  break;
                case CTRL_SLEEP_TIMER:
                  AudioHandler::setSleepTimer(card.value);
                  break;
                default:
                  break;
              }
            } else {
              // Dodaj do kolejki
              addToQueue(card);
              
              // Jeśli to pierwsza karta, zacznij odtwarzanie
              if (getQueueSize() == 1) {
                AudioHandler::play(card.path);
              }
            }
          } else {
            Serial.printf("[RFID] UWAGA: Brak konfiguracji dla karty: %s\n", uid.c_str());
            addDisplayLog("Nieznana karta!");
          }
        }
      } else {
        // Brak karty - sprawdź czy jakieś karty zostały usunięte
        std::vector<String> removedCards;
        
        for (const String& readerUID : cardsInReader) {
          // Sprawdź czy karta nadal jest w czytniku
          bool stillPresent = false;
          
          // Szybkie ponowne skanowanie
          uint8_t uidBuffer[7];
          uint8_t uidLength;
          if (nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuffer, &uidLength, 50)) {
            String currentUID = "";
            for (uint8_t i = 0; i < uidLength; i++) {
              if (currentUID.length() > 0) currentUID += ":";
              if (uidBuffer[i] < 0x10) currentUID += "0";
              currentUID += String(uidBuffer[i], HEX);
            }
            currentUID.toUpperCase();
            
            if (currentUID == readerUID) {
              stillPresent = true;
            }
          }
          
          if (!stillPresent) {
            removedCards.push_back(readerUID);
          }
        }
        
        // Usuń karty z listy
        for (const String& removedUID : removedCards) {
          Serial.printf("[RFID] Usunięto kartę: %s\n", removedUID.c_str());
          
          // Usuń z czynika
          for (auto it = cardsInReader.begin(); it != cardsInReader.end(); ++it) {
            if (*it == removedUID) {
              cardsInReader.erase(it);
              break;
            }
          }
          
          // Znajdź kartę w konfiguracji
          RFIDCard card = ConfigManager::findCard(removedUID);
          if (card.uid.length() > 0) {
            if (card.type == CARD_CTRL) {
              // Wyłącz funkcję sterującą
              switch (card.action) {
                case CTRL_REPEAT_3:
                case CTRL_REPEAT_5:
                case CTRL_REPEAT_INF:
                  AudioHandler::setRepeat(0);
                  audioState.repeat = false;
                  addDisplayLog("Repeat OFF");
                  break;
                case CTRL_SHUFFLE_ON:
                  AudioHandler::setShuffle(false);
                  break;
                case CTRL_SLEEP_TIMER:
                  AudioHandler::setSleepTimer(0);
                  break;
                default:
                  break;
              }
            } else {
              // Usuń z kolejki
              removeFromQueue(removedUID);
              
              // Jeśli to była jedyna karta, zatrzymaj odtwarzanie
              if (getQueueSize() == 0) {
                AudioHandler::stop();
              }
            }
          }
        }
      }
      
      // Sprawdź timeout skanowania Web UI
      if (webScanMode) {
        if (xSemaphoreTake(scanMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
          if (millis() - webScanStartTime > WEB_SCAN_TIMEOUT_MS) {
            Serial.println("[RFID] Timeout skanowania Web UI");
            webScanMode = false;
          }
          xSemaphoreGive(scanMutex);
        }
      }
      
      vTaskDelay(pdMS_TO_TICKS(RFID_SCAN_INTERVAL_MS));
    }
  }
  
  // ========================================================================
  // SKANOWANIE
  // ========================================================================
  
  String scanCard() {
    if (!nfc) return "";
    
    uint8_t uid[7];
    uint8_t uidLength;
    
    // Szybkie skanowanie (50ms timeout)
    if (nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50)) {
      String uidStr = "";
      for (uint8_t i = 0; i < uidLength; i++) {
        if (uidStr.length() > 0) uidStr += ":";
        if (uid[i] < 0x10) uidStr += "0";
        uidStr += String(uid[i], HEX);
      }
      uidStr.toUpperCase();
      return uidStr;
    }
    
    return "";
  }
  
  String getLastScanned() {
    String uid = "";
    if (xSemaphoreTake(scanMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      uid = lastScannedUID;
      lastScannedUID = "";  // Wyczyść po odczytaniu
      xSemaphoreGive(scanMutex);
    }
    return uid;
  }
  
  void startWebScan() {
    if (xSemaphoreTake(scanMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      webScanMode = true;
      webScanStartTime = millis();
      lastScannedUID = "";
      xSemaphoreGive(scanMutex);
    }
    Serial.println("[RFID] Rozpoczęto skanowanie dla Web UI");
  }
  
  void stopWebScan() {
    if (xSemaphoreTake(scanMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      webScanMode = false;
      xSemaphoreGive(scanMutex);
    }
    Serial.println("[RFID] Zatrzymano skanowanie dla Web UI");
  }
  
  bool isWebScanning() {
    bool scanning = false;
    if (xSemaphoreTake(scanMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      scanning = webScanMode;
      xSemaphoreGive(scanMutex);
    }
    return scanning;
  }
  
  // ========================================================================
  // KOLEJKA ODTWARZANIA
  // ========================================================================
  
  void addToQueue(const RFIDCard& card) {
    if (xSemaphoreTake(queueMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      // Sprawdź czy już jest w kolejce
      bool exists = false;
      for (int i = 0; i < MAX_QUEUE_SIZE; i++) {
        if (playQueue[i].active && playQueue[i].card.uid == card.uid) {
          exists = true;
          break;
        }
      }
      
      if (!exists) {
        // Znajdź wolne miejsce
        for (int i = 0; i < MAX_QUEUE_SIZE; i++) {
          if (!playQueue[i].active) {
            playQueue[i].card = card;
            playQueue[i].active = true;
            playQueue[i].addedTime = millis();
            queueSize++;
            Serial.printf("[RFID] Dodano do kolejki: %s (rozmiar: %d)\n", card.uid.c_str(), queueSize);
            addDisplayLog("Kolejka: +" + card.path);
            break;
          }
        }
        
        if (queueSize >= MAX_QUEUE_SIZE) {
          Serial.println("[RFID] UWAGA: Kolejka pełna!");
          addDisplayLog("Kolejka pełna!");
        }
      }
      
      xSemaphoreGive(queueMutex);
    }
  }
  
  void removeFromQueue(const String& uid) {
    if (xSemaphoreTake(queueMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      for (int i = 0; i < MAX_QUEUE_SIZE; i++) {
        if (playQueue[i].active && playQueue[i].card.uid == uid) {
          playQueue[i].active = false;
          queueSize--;
          Serial.printf("[RFID] Usunięto z kolejki: %s (rozmiar: %d)\n", uid.c_str(), queueSize);
          addDisplayLog("Kolejka: -" + uid);
          break;
        }
      }
      xSemaphoreGive(queueMutex);
    }
  }
  
  bool isInQueue(const String& uid) {
    bool inQueue = false;
    if (xSemaphoreTake(queueMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      for (int i = 0; i < MAX_QUEUE_SIZE; i++) {
        if (playQueue[i].active && playQueue[i].card.uid == uid) {
          inQueue = true;
          break;
        }
      }
      xSemaphoreGive(queueMutex);
    }
    return inQueue;
  }
  
  void clearQueue() {
    if (xSemaphoreTake(queueMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      for (int i = 0; i < MAX_QUEUE_SIZE; i++) {
        playQueue[i].active = false;
      }
      queueSize = 0;
      Serial.println("[RFID] Wyczyszczono kolejkę");
      xSemaphoreGive(queueMutex);
    }
  }
  
  int getQueueSize() {
    int size = 0;
    if (xSemaphoreTake(queueMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      size = queueSize;
      xSemaphoreGive(queueMutex);
    }
    return size;
  }
}
