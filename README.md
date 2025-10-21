# BajkoGrajek - RFID MP3 Player

Profesjonalny odtwarzacz MP3 sterowany kartami RFID z interfejsem WWW, przeznaczony dla dzieci.

## 🎵 Funkcje

### Podstawowe
- **Odtwarzanie MP3** - wysokiej jakości audio przez I2S DAC
- **Karty RFID** - przypisywanie piosenek i katalogów do kart NFC/RFID
- **Fizyczna playlista** - do 10 kart jednocześnie na czytniku
- **Automatyczne zarządzanie** - dodanie karty = start, usunięcie = stop
- **Wyświetlacz OLED** - status, głośność, kolejka, logi
- **4 przyciski** - głośność, play, pause/stop

### Karty sterujące
- **REPEAT** - powtarzanie 3x, 5x lub w nieskończoność
- **SHUFFLE** - losowa kolejność odtwarzania
- **SLEEP TIMER** - automatyczne wyłączenie po zadanym czasie

### Interfejs WWW
- **Responsywny design** - działa na telefonie, tablecie, komputerze
- **Przeglądarka plików** - katalogi i pliki MP3 na karcie SD
- **Upload plików** - wysyłanie MP3 przez przeglądarkę
- **Przypisywanie kart** - skanowanie i konfiguracja kart RFID
- **Konfiguracja WiFi** - łączenie z siecią domową
- **Status na żywo** - aktualizowany co 2 sekundy

### Serwer FTP
- **Pełny dostęp do SD** - zarządzanie plikami przez FTP
- **SimpleFTPServer** - stabilny i szybki transfer
- **Dane dostępu** - user: `bajko`, hasło: `grajek`

### OTA Update
- **Zdalna aktualizacja** - firmware przez przeglądarkę
- **Bez kabla** - update przez WiFi

## 📦 Wymagane komponenty

### Hardware
- **ESP32** - dowolny model z WiFi
- **Karta SD** - do przechowywania plików MP3
- **I2S DAC** - np. MAX98357, PCM5102
- **PN532** - czytnik RFID/NFC (I2C)
- **OLED SSD1306** - wyświetlacz 128x32 (I2C)
- **4 przyciski** - tact switch z pull-up
- **Buzzer** (opcjonalny) - dla sygnału resetu WiFi

### Pinout

```
SD Card (SPI):
- CS:   GPIO 5
- SCK:  GPIO 18
- MISO: GPIO 19
- MOSI: GPIO 23

I2S Audio:
- DOUT: GPIO 25
- BCLK: GPIO 26
- LRC:  GPIO 22

I2C OLED (Wire):
- SDA:  GPIO 21
- SCL:  GPIO 22

I2C RFID (Wire1):
- SDA:  GPIO 33
- SCL:  GPIO 32

Przyciski:
- VOL-:  GPIO 17
- PAUSE: GPIO 13
- PLAY:  GPIO 4
- VOL+:  GPIO 27

Buzzer (opcjonalny):
- BUZZER: GPIO 15
```

## 📚 Wymagane biblioteki

Zainstaluj następujące biblioteki przez Arduino Library Manager:

```
- ESP8266Audio (https://github.com/earlephilhower/ESP8266Audio)
- Adafruit_PN532
- Adafruit_SSD1306
- Adafruit_GFX
- ArduinoJson (v6)
- ESPAsyncWebServer (https://github.com/me-no-dev/ESPAsyncWebServer)
- AsyncTCP (https://github.com/me-no-dev/AsyncTCP)
- SimpleFTPServer (https://github.com/xreef/SimpleFTPServer)
```

Biblioteki wbudowane (nie trzeba instalować):
- SD, SPI, Wire, WiFi, Preferences

## 🚀 Instalacja

### 1. Arduino IDE

1. Zainstaluj Arduino IDE (wersja 1.8.x lub 2.x)
2. Dodaj obsługę ESP32:
   - Otwórz `Preferences`
   - W polu `Additional Boards Manager URLs` dodaj:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Otwórz `Tools > Board > Boards Manager`
   - Wyszukaj `ESP32` i zainstaluj

3. Zainstaluj wymagane biblioteki (patrz sekcja wyżej)

4. Otwórz plik `BajkoGrajek.ino`

5. Wybierz swoją płytkę:
   - `Tools > Board > ESP32 Arduino > ESP32 Dev Module`

6. Skonfiguruj opcje:
   - `Upload Speed: 921600`
   - `Flash Frequency: 80MHz`
   - `Flash Mode: QIO`
   - `Flash Size: 4MB (32Mb)`
   - `Partition Scheme: Default 4MB with spiffs`
   - `Core Debug Level: None`

7. Wgraj kod: `Sketch > Upload`

### 2. PlatformIO

1. Utwórz plik `platformio.ini`:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

monitor_speed = 115200
upload_speed = 921600

lib_deps = 
    ESP8266Audio
    adafruit/Adafruit PN532
    adafruit/Adafruit SSD1306
    adafruit/Adafruit GFX Library
    bblanchon/ArduinoJson @ ^6.21.3
    https://github.com/me-no-dev/ESPAsyncWebServer
    https://github.com/me-no-dev/AsyncTCP
    https://github.com/xreef/SimpleFTPServer
```

2. Wgraj kod: `pio run --target upload`

## 🎮 Użytkowanie

### Pierwsze uruchomienie

1. **Włącz urządzenie** - ESP32 uruchomi się w trybie Access Point
2. **Podłącz się do WiFi** - szukaj sieci `BajkoGrajek` (bez hasła)
3. **Otwórz przeglądarkę** - wejdź na adres `192.168.4.1`
4. **Konfiguruj WiFi** (opcjonalnie):
   - Przejdź do sekcji "Konfiguracja WiFi"
   - Wpisz nazwę sieci i hasło
   - Kliknij "Zapisz i restart"
   - Po restarcie urządzenie połączy się z siecią domową

### Reset WiFi

Jeśli chcesz wrócić do trybu Access Point:

1. **Naciśnij i przytrzymaj** przyciski VOL- i VOL+ (1 i 4)
2. **Włącz urządzenie** (lub naciśnij reset)
3. **Usłyszysz sygnał dźwiękowy** (5kHz przez 1s) - jeśli podłączony buzzer
4. **Urządzenie się zrestartuje** w trybie AP

### Przygotowanie karty SD

1. Sformatuj kartę SD jako **FAT32**
2. Utwórz strukturę katalogów, np.:
   ```
   /
   ├── bajki/
   │   ├── bajka1.mp3
   │   └── bajka2.mp3
   ├── piosenki/
   │   ├── piosenka1.mp3
   │   └── piosenka2.mp3
   └── kolysanki/
       └── kolysanka.mp3
   ```
3. Włóż kartę do ESP32

### Przypisywanie kart RFID

**Przez interfejs WWW:**

1. Przejdź do sekcji "Karty RFID"
2. Kliknij "Skanuj kartę"
3. Przyłóż kartę do czytnika
4. Wybierz typ karty:
   - **Pojedynczy plik** - odtwarza jeden plik MP3
   - **Katalog** - odtwarza wszystkie MP3 z katalogu
   - **Karta sterująca** - REPEAT, SHUFFLE, SLEEP_TIMER
5. Wpisz ścieżkę lub wybierz akcję
6. Kliknij "Przypisz"

**Ręcznie (config.csv):**

Plik `/config.csv` na karcie SD:
```csv
# Format: UID;typ;ścieżka_lub_akcja

# Pojedynczy plik
04:A1:B2:C3;file;/bajki/bajka1.mp3

# Katalog
04:D4:E5:F6;dir;/piosenki

# Karty sterujące
04:11:22:33;ctrl;REPEAT3
04:44:55:66;ctrl;SHUFFLE_ON
04:77:88:99;ctrl;SLEEP_TIMER:30
```

### Sterowanie przyciskami

- **VOL-** (GPIO 17) - zmniejsz głośność o 10%
- **PAUSE** (GPIO 13) - pauza/wznów (2x = stop)
- **PLAY** (GPIO 4) - odtwórz/następny
- **VOL+** (GPIO 27) - zwiększ głośność o 10%

### FTP

Połącz się z serwerem FTP:
```
Host: <IP urządzenia>
Port: 21
User: bajko
Pass: grajek
```

Możesz używać dowolnego klienta FTP (FileZilla, WinSCP, itp.)

## 🛠️ Konfiguracja

### Zmiana pinów

Edytuj plik `src/config.h`:

```cpp
// Przykład zmiany pinów I2S
#define I2S_DOUT    25
#define I2S_BCLK    26
#define I2S_LRC     22
```

### Zmiana nazwy AP

Edytuj plik `src/config.h`:

```cpp
#define AP_SSID             "MójGrajek"
#define AP_PASSWORD         "haslo123"  // Lub "" dla braku hasła
```

### Zmiana danych FTP

Edytuj plik `src/config.h`:

```cpp
#define FTP_USER            "admin"
#define FTP_PASS            "password"
```

## 🏗️ Struktura projektu

```
BajkoGrajek-RFID-MP3-Player/
├── BajkoGrajek.ino              # Główny plik (setup + loop)
├── README.md                     # Ten plik
├── libraries.txt                 # Lista bibliotek
└── src/
    ├── config.h                  # Definicje, piny, struktury
    ├── globals.cpp               # Zmienne globalne
    ├── audio_handler.h/.cpp      # Obsługa ESP8266Audio, I2S
    ├── rfid_handler.h/.cpp       # Obsługa PN532, kolejka
    ├── display_handler.h/.cpp    # Obsługa OLED SSD1306
    ├── button_handler.h/.cpp     # Obsługa 4 przycisków
    ├── web_server.h/.cpp         # ESPAsyncWebServer + HTML UI
    ├── ftp_server.h/.cpp         # SimpleFTPServer
    ├── config_manager.h/.cpp     # Zarządzanie config.csv
    └── wifi_manager.h/.cpp       # Zarządzanie WiFi AP/STA
```

## 🔧 Rozwiązywanie problemów

### Karta SD nie jest wykrywana

- Sprawdź połączenia SPI
- Upewnij się, że karta jest sformatowana jako FAT32
- Sprawdź czy karta jest kompatybilna (max 32GB dla FAT32)

### RFID nie działa

- Sprawdź połączenia I2C (Wire1)
- Upewnij się, że PN532 jest w trybie I2C (przełączniki)
- Sprawdź adres I2C (domyślnie 0x24)

### Brak dźwięku

- Sprawdź połączenia I2S
- Upewnij się, że DAC jest prawidłowo podłączony
- Sprawdź czy głośność nie jest ustawiona na 0
- Testuj z różnymi plikami MP3

### Nie można połączyć z WiFi

- Reset WiFi: przyciski VOL- + VOL+ przy starcie
- Sprawdź poprawność danych sieci
- Sprawdź zasięg sygnału WiFi

### OTA Update nie działa

- Upewnij się, że plik .bin jest prawidłowy
- Sprawdź czy ESP32 ma wystarczająco pamięci
- Spróbuj zmniejszyć rozmiar firmware (wyłącz debug)

## 📄 Licencja

MIT License - możesz swobodnie używać, modyfikować i rozpowszechniać ten projekt.

## 👥 Autorzy

BajkoGrajek Team

## 🤝 Wkład

Pull requesty są mile widziane! W przypadku większych zmian, najpierw otwórz issue.

## 📞 Wsparcie

W razie problemów otwórz issue na GitHubie.

---

**Enjoy your music! 🎵**