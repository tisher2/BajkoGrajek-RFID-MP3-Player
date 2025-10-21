# BajkoGrajek - Struktura projektu

## 📁 Organizacja plików

```
BajkoGrajek-RFID-MP3-Player/
├── BajkoGrajek.ino              (2.5K)  - Główny plik Arduino
├── platformio.ini               (1.8K)  - Konfiguracja PlatformIO
├── .gitignore                           - Pliki ignorowane przez Git
│
├── README.md                    (10K)   - Główna dokumentacja
├── QUICKSTART.md                (6.2K)  - Szybki start
├── WIRING.md                    (9.1K)  - Schemat połączeń
├── libraries.txt                (3.9K)  - Lista bibliotek
├── PROJECT_STRUCTURE.md                 - Ten plik
│
└── src/                                 - Moduły źródłowe
    ├── config.h                 (5.8K)  - Definicje, stałe, struktury
    ├── globals.cpp              (776B)  - Zmienne globalne
    │
    ├── audio_handler.h          (801B)  - Interface audio
    ├── audio_handler.cpp        (10K)   - Implementacja audio
    │
    ├── rfid_handler.h           (633B)  - Interface RFID
    ├── rfid_handler.cpp         (12K)   - Implementacja RFID
    │
    ├── display_handler.h        (484B)  - Interface wyświetlacza
    ├── display_handler.cpp      (5.0K)  - Implementacja wyświetlacza
    │
    ├── button_handler.h         (390B)  - Interface przycisków
    ├── button_handler.cpp       (5.0K)  - Implementacja przycisków
    │
    ├── web_server.h             (275B)  - Interface serwera WWW
    ├── web_server.cpp           (27K)   - Implementacja serwera + HTML
    │
    ├── ftp_server.h             (239B)  - Interface serwera FTP
    ├── ftp_server.cpp           (1.1K)  - Implementacja serwera FTP
    │
    ├── config_manager.h         (451B)  - Interface zarządzania konfiguracją
    ├── config_manager.cpp       (6.8K)  - Implementacja zarządzania konfiguracją
    │
    ├── wifi_manager.h           (532B)  - Interface WiFi
    └── wifi_manager.cpp         (4.4K)  - Implementacja WiFi

Razem: ~82KB kodu źródłowego, ~3941 linii
```

## 🔧 Moduły funkcjonalne

### 1. Core System (config.h, globals.cpp, BajkoGrajek.ino)

**Odpowiedzialność:**
- Definicje pinów i stałych systemowych
- Struktury danych (RFIDCard, QueueItem, AudioState)
- Zmienne globalne i mutexy
- Główna pętla setup() i loop()

**Kluczowe elementy:**
- MAX_QUEUE_SIZE = 10
- SPI_FREQUENCY = 40MHz
- I2C_FREQUENCY = 400kHz
- FreeRTOS Queues i Semaphores

### 2. Audio Handler (audio_handler.h/.cpp)

**Odpowiedzialność:**
- Obsługa ESP8266Audio
- Dekodowanie MP3
- Streaming I2S
- Zarządzanie playlistami
- Komendy audio (play, pause, stop, volume)

**FreeRTOS Task:**
- Core: 1 (dedykowany dla audio)
- Priorytet: 10 (wysoki)
- Stack: 8192 bytes
- Częstotliwość: ~100Hz (10ms loop)

**Funkcje:**
- `init()` - inicjalizacja I2S, SD, volume
- `play(path)` - odtwórz plik/katalog
- `playNext()` - następny z playlisty
- `setVolume(vol)` - 0-100%
- `setRepeat(count)` - powtarzanie
- `setShuffle(enabled)` - losowa kolejność
- `setSleepTimer(min)` - timer wyłączenia

### 3. RFID Handler (rfid_handler.h/.cpp)

**Odpowiedzialność:**
- Obsługa PN532 przez I2C
- Skanowanie kart NFC/RFID
- Wykrywanie dodania/usunięcia kart
- Zarządzanie kolejką odtwarzania (max 10 kart)
- Tryb skanowania dla Web UI

**FreeRTOS Task:**
- Core: 0
- Priorytet: 5 (średni)
- Stack: 4096 bytes
- Częstotliwość: ~5Hz (200ms interval)

**Funkcje:**
- `init()` - inicjalizacja PN532 I2C
- `scanCard()` - skanowanie karty
- `addToQueue(card)` - dodaj do kolejki
- `removeFromQueue(uid)` - usuń z kolejki
- `startWebScan()` - tryb dla Web UI

**Anti-debounce:**
- 1000ms między kolejnymi odczytami tej samej karty
- Automatyczne wykrywanie usunięcia karty

### 4. Display Handler (display_handler.h/.cpp)

**Odpowiedzialność:**
- Obsługa OLED SSD1306 128x32
- Wyświetlanie statusu systemu
- Scrollowanie logów
- Pasek postępu (opcjonalny)

**FreeRTOS Task:**
- Core: 0
- Priorytet: 2 (niski)
- Stack: 4096 bytes
- Częstotliwość: ~10Hz (100ms update)

**Layout ekranu:**
```
┌─────────────────────────┐
│ piosenka.mp3           │  Linia 1: Nazwa pliku
│ Vol:75% Q:3 [R:5]      │  Linia 2: Status
│ Log: Dodano kartę      │  Linia 3: Log 1
│ Vol: 75%               │  Linia 4: Log 2
└─────────────────────────┘
```

**Funkcje:**
- `init()` - inicjalizacja OLED I2C
- `update()` - odświeżenie wyświetlacza
- `showLog(msg)` - dodaj log

### 5. Button Handler (button_handler.h/.cpp)

**Odpowiedzialność:**
- Obsługa 4 przycisków
- Debouncing (50ms)
- Detekcja pojedynczego/podwójnego kliknięcia
- Reset WiFi (kombinacja przycisków 1+4)

**FreeRTOS Task:**
- Core: 0
- Priorytet: 3 (średni)
- Stack: 2048 bytes
- Częstotliwość: ~50Hz (20ms check)

**Przyciski:**
- VOL- (GPIO 17): zmniejsz głośność -10%
- PAUSE (GPIO 13): pauza/wznów, 2x = stop
- PLAY (GPIO 4): odtwórz/następny
- VOL+ (GPIO 27): zwiększ głośność +10%

**Funkcje:**
- `init()` - pinMode INPUT_PULLUP
- `isResetWiFiPressed()` - sprawdź kombinację 1+4

### 6. Web Server (web_server.h/.cpp)

**Odpowiedzialność:**
- ESPAsyncWebServer na porcie 80
- Responsywny HTML UI z gradientem
- RESTful API (JSON)
- Upload plików MP3 (atomic)
- OTA firmware update

**Endpointy:**
- `GET /` - strona główna HTML
- `GET /api/status` - status JSON
- `GET /api/files?path=` - lista plików
- `POST /api/scan-rfid` - start skanowania
- `GET /api/scan-rfid-status` - status skanowania
- `POST /api/assign-rfid` - przypisz kartę
- `POST /api/play` - odtwórz plik
- `POST /api/delete` - usuń plik
- `POST /api/upload` - upload MP3
- `POST /api/wifi` - konfiguracja WiFi
- `GET /ota` - strona OTA
- `POST /update` - upload firmware

**UI Features:**
- Gradient background (purple-blue)
- Glass morphism cards
- Real-time status updates (2s interval)
- File browser z breadcrumb navigation
- RFID scanner z timeout
- Control cards assignment
- WiFi configuration
- FTP info
- localStorage preferences

### 7. FTP Server (ftp_server.h/.cpp)

**Odpowiedzialność:**
- SimpleFTPServer na porcie 21
- Pełny dostęp do karty SD
- Upload/download/delete plików

**Konfiguracja:**
- User: `bajko`
- Pass: `grajek`
- Port: 21

**Funkcje:**
- `init()` - inicjalizacja serwera
- `handle()` - obsługa (wywołaj w loop)

### 8. Config Manager (config_manager.h/.cpp)

**Odpowiedzialność:**
- Zarządzanie plikiem /config.csv na SD
- Parsowanie konfiguracji kart RFID
- Atomic write operations
- Auto-tworzenie pustego pliku

**Format CSV:**
```csv
# Komentarz
UID;type;path_or_action

# Przykłady:
04:A1:B2:C3;file;/muzyka/piosenka.mp3
04:D4:E5:F6;dir;/muzyka/album
04:11:22:33;ctrl;REPEAT3
04:44:55:66;ctrl;SHUFFLE_ON
04:77:88:99;ctrl;SLEEP_TIMER:30
```

**Funkcje:**
- `load()` - wczytaj konfigurację
- `save()` - zapisz (atomic)
- `addCard(card)` - dodaj/zaktualizuj
- `removeCard(uid)` - usuń
- `findCard(uid)` - znajdź po UID

### 9. WiFi Manager (wifi_manager.h/.cpp)

**Odpowiedzialność:**
- Zarządzanie WiFi AP/STA
- Preferences (NVS storage)
- Auto-reconnect
- Reset konfiguracji

**Tryb AP:**
- SSID: `BajkoGrajek`
- Pass: brak
- IP: `192.168.4.1`

**Tryb STA:**
- Połączenie z siecią domową
- DHCP lub static IP
- Auto-reconnect przy utracie połączenia

**Funkcje:**
- `init()` - start AP lub STA
- `reset()` - wyczyść dane i restart
- `connect(ssid, pass)` - połącz z siecią
- `saveCredentials()` - zapisz do NVS
- `getIP()` - pobierz adres IP

## 🔄 Przepływ danych

### Inicjalizacja (setup):
```
ButtonHandler::init()
    ↓
sprawdź reset WiFi
    ↓
DisplayHandler::init()
    ↓
AudioHandler::init()
    ↓
RFIDHandler::init()
    ↓
ConfigManager::load()
    ↓
WiFiManager::init()
    ↓
WebServer::init()
    ↓
FTPServer::init()
    ↓
create FreeRTOS tasks
```

### Skanowanie karty:
```
RFID Task (Core 0, 200ms)
    ↓
scanCard()
    ↓
anti-debounce check
    ↓
ConfigManager::findCard()
    ↓
[karta znaleziona?]
    ↓
add to queue (mutex)
    ↓
Audio Task: play()
    ↓
Display Task: update status
```

### Odtwarzanie audio:
```
Audio Task (Core 1, 10ms)
    ↓
check audioCommandQueue
    ↓
mp3->loop() (dekodowanie)
    ↓
I2S streaming (DMA)
    ↓
[koniec pliku?]
    ↓
check repeat/shuffle
    ↓
playNext() lub stop()
```

### Web UI:
```
Przeglądarka
    ↓
HTTP Request
    ↓
ESPAsyncWebServer (Core 1)
    ↓
Handler function
    ↓
access SD/config (mutex)
    ↓
JSON Response
    ↓
update UI (JavaScript)
```

## 🧵 FreeRTOS Tasks

| Task | Core | Priorytet | Stack | Częstotliwość |
|------|------|-----------|-------|---------------|
| audioTask | 1 | 10 | 8192 | 100Hz (10ms) |
| rfidTask | 0 | 5 | 4096 | 5Hz (200ms) |
| buttonTask | 0 | 3 | 2048 | 50Hz (20ms) |
| displayTask | 0 | 2 | 4096 | 10Hz (100ms) |

**Mutexes:**
- sdMutex - dostęp do karty SD
- displayMutex - dostęp do OLED
- queueMutex - kolejka odtwarzania
- logMutex - logi dla OLED
- scanMutex - tryb skanowania Web UI

**Queues:**
- audioCommandQueue - komendy dla audio task

## 💾 Wykorzystanie pamięci

### Flash (przybliżone):
- Kod programu: ~500KB
- Biblioteki: ~600KB
- HTML/CSS inline: ~20KB
- **Razem: ~1.1MB** (z 4MB dostępne)

### RAM (przybliżone):
- Statyczne zmienne: ~30KB
- FreeRTOS tasks: ~20KB (stosy)
- Bufory audio: ~8KB
- Bufory sieciowe: ~16KB
- **Razem: ~74KB** (z 320KB dostępne)

### SD Card:
- config.csv: <10KB
- Pliki MP3: bez limitu (max 32GB dla FAT32)

## 🛠️ Konfiguracja build

### Arduino IDE:
```
Board: ESP32 Dev Module
Upload Speed: 921600
CPU Frequency: 240MHz
Flash Frequency: 80MHz
Flash Mode: QIO
Flash Size: 4MB (32Mb)
Partition Scheme: Default 4MB with spiffs
PSRAM: Disabled (lub Enabled jeśli dostępny)
Core Debug Level: None
```

### PlatformIO:
```ini
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600
```

## 📊 Statystyki kodu

| Kategoria | Linie | Procent |
|-----------|-------|---------|
| Kod C++ | ~2800 | 71% |
| Komentarze | ~500 | 13% |
| HTML/CSS/JS | ~600 | 15% |
| Puste linie | ~41 | 1% |
| **Razem** | **3941** | **100%** |

### Rozkład per moduł:
- web_server.cpp: 27KB (największy - zawiera HTML)
- rfid_handler.cpp: 12KB
- audio_handler.cpp: 10KB
- config_manager.cpp: 6.8KB
- config.h: 5.8KB
- display_handler.cpp: 5.0KB
- button_handler.cpp: 5.0KB
- wifi_manager.cpp: 4.4KB
- BajkoGrajek.ino: 2.5KB
- Pozostałe: <1KB każdy

## 🔐 Bezpieczeństwo

### Uwagi bezpieczeństwa:
1. **WiFi AP bez hasła** - tylko dla rozwoju, ustaw hasło w produkcji
2. **FTP plain text** - nie używaj przez Internet (tylko LAN)
3. **Web bez HTTPS** - brak szyfrowania (lokalnie OK)
4. **Brak autentykacji Web** - każdy w sieci ma dostęp

### Rekomendacje dla produkcji:
- Ustaw hasło dla AP w config.h
- Użyj WPA2 dla WiFi STA
- Implementuj podstawową autentykację HTTP
- Rozważ HTTPS (wymaga certyfikatu)
- Zmień domyślne dane FTP

## 📝 Konwencje kodowania

### Nazewnictwo:
- **Funkcje:** camelCase (np. `playNext()`)
- **Klasy/Namespace:** PascalCase (np. `AudioHandler`)
- **Stałe:** UPPER_SNAKE_CASE (np. `MAX_QUEUE_SIZE`)
- **Zmienne:** camelCase (np. `queueSize`)
- **Pliki:** snake_case (np. `audio_handler.cpp`)

### Komentarze:
- Język: **polski**
- Format: `// komentarz` lub `/* blok */`
- Sekcje: `// ===== SEKCJA =====`

### Struktura pliku .cpp:
```cpp
#include "header.h"
// inne includes

namespace ModuleName {
  // zmienne statyczne
  
  // ===== INICJALIZACJA =====
  void init() { ... }
  
  // ===== FREERTOS TASK =====
  void createTask() { ... }
  void task() { ... }
  
  // ===== FUNKCJE PUBLICZNE =====
  void publicFunction() { ... }
  
  // ===== FUNKCJE POMOCNICZE =====
  static void privateFunction() { ... }
}
```

## 🚀 Dalszy rozwój

### Możliwe rozszerzenia:
1. **Bluetooth** - sterowanie przez telefon
2. **MQTT** - integracja z Home Assistant
3. **Wbudowany wzmacniacz** - bez zewnętrznego DAC
4. **Ekran dotykowy** - zamiast przycisków
5. **Większy OLED** - graficzny UI
6. **RTC** - zegar czasu rzeczywistego
7. **Batteria** - przenośna wersja
8. **Mikrofon** - nagrywanie głosu
9. **Equalizer** - regulacja tonów
10. **Playlist editor** - Web UI do zarządzania playlistami

### Planowane poprawki:
- [ ] Progress bar na OLED
- [ ] Więcej formatów audio (FLAC, AAC)
- [ ] Obsługa ID3 tags
- [ ] Bookmarks (wznowienie od miejsca zatrzymania)
- [ ] Sleep fade out
- [ ] Alarm/timer
- [ ] Multi-language support

---

*Dokument zaktualizowany: 2025*
*Wersja projektu: 1.0.0*
