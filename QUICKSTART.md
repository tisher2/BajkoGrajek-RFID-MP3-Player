# BajkoGrajek - Szybki start

## ⚡ Szybka instalacja (5 minut)

### 1. Przygotuj hardware

Podłącz komponenty zgodnie z pinoutem:

| Komponent | Pin ESP32 | Opis |
|-----------|-----------|------|
| SD CS | GPIO 5 | Chip Select karty SD |
| SD SCK | GPIO 18 | SPI Clock |
| SD MISO | GPIO 19 | SPI MISO |
| SD MOSI | GPIO 23 | SPI MOSI |
| I2S DOUT | GPIO 25 | Audio Data Out |
| I2S BCLK | GPIO 26 | Audio Bit Clock |
| I2S LRC | GPIO 22 | Audio Left/Right Clock |
| OLED SDA | GPIO 21 | I2C Data |
| OLED SCL | GPIO 22 | I2C Clock |
| RFID SDA | GPIO 33 | I2C Data (Wire1) |
| RFID SCL | GPIO 32 | I2C Clock (Wire1) |
| BTN VOL- | GPIO 17 | Przycisk (pull-up) |
| BTN PAUSE | GPIO 13 | Przycisk (pull-up) |
| BTN PLAY | GPIO 4 | Przycisk (pull-up) |
| BTN VOL+ | GPIO 27 | Przycisk (pull-up) |

**Uwaga:** Przyciski podłącz z pull-up (wewnętrzny INPUT_PULLUP)

### 2. Zainstaluj oprogramowanie

**Opcja A: Arduino IDE**

1. Zainstaluj Arduino IDE 2.x
2. Dodaj ESP32: `Preferences` → `Additional Boards Manager URLs`:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Zainstaluj biblioteki przez Library Manager:
   - ESP8266Audio
   - Adafruit PN532
   - Adafruit SSD1306
   - Adafruit GFX Library
   - ArduinoJson (v6)
   - SimpleFTPServer

4. Zainstaluj ręcznie:
   - [AsyncTCP](https://github.com/me-no-dev/AsyncTCP/archive/master.zip)
   - [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip)

**Opcja B: PlatformIO** (zalecane)

```bash
# Sklonuj repozytorium
git clone https://github.com/tisher2/BajkoGrajek-RFID-MP3-Player.git
cd BajkoGrajek-RFID-MP3-Player

# Zbuduj i wgraj
pio run --target upload
```

### 3. Przygotuj kartę SD

1. Sformatuj kartę SD jako **FAT32**
2. Wgraj przykładowe pliki MP3:
   ```
   /
   ├── test.mp3
   └── muzyka/
       └── piosenka.mp3
   ```
3. Włóż kartę do ESP32

### 4. Wgraj firmware

**Arduino IDE:**
- Wybierz Board: `ESP32 Dev Module`
- Port: wybierz właściwy port COM/tty
- Upload Speed: `921600`
- Kliknij `Upload`

**PlatformIO:**
```bash
pio run --target upload
```

### 5. Pierwsze uruchomienie

1. **Otwórz Serial Monitor** (115200 baud)
2. **Obserwuj logi inicjalizacji** - powinny pojawić się:
   ```
   [Audio] SD OK
   [RFID] RFID OK
   [Display] OLED zainicjalizowany
   [WiFi] AP SSID: BajkoGrajek
   [WiFi] AP IP: 192.168.4.1
   [Web] Serwer WWW uruchomiony
   [FTP] Serwer FTP uruchomiony
   ```

3. **Połącz się z WiFi:**
   - SSID: `BajkoGrajek`
   - Hasło: brak
   
4. **Otwórz przeglądarkę:**
   - Adres: `http://192.168.4.1`

## 🎮 Podstawowe operacje

### Przypisanie pierwszej karty

1. W przeglądarce kliknij **"Skanuj kartę"**
2. Przyłóż kartę RFID do czytnika
3. Poczekaj na wyświetlenie UID
4. Wybierz typ: **"Pojedynczy plik"**
5. Wpisz: `/test.mp3`
6. Kliknij **"Przypisz"**

### Test odtwarzania

1. **Przyłóż kartę** do czytnika
2. **Powinno się rozpocząć odtwarzanie** - sprawdź:
   - Serial Monitor: `[Audio] Odtwarzanie: /test.mp3`
   - OLED: nazwa pliku i głośność
   - Głośnik: dźwięk

3. **Odbierz kartę** - odtwarzanie powinno się zatrzymać

### Test przycisków

- **VOL+** - zwiększ głośność (sprawdź OLED)
- **VOL-** - zmniejsz głośność
- **PAUSE** - pauza (1x), stop (2x)
- **PLAY** - rozpocznij/następny

## 🔧 Najczęstsze problemy

### SD nie działa

```
[Audio] BŁĄD: Nie można zainicjalizować karty SD!
```

**Rozwiązanie:**
1. Sprawdź połączenia SPI
2. Upewnij się, że karta jest FAT32
3. Spróbuj innej karty SD
4. Zmniejsz częstotliwość SPI w config.h (z 40MHz na 20MHz)

### RFID nie wykrywa kart

```
[RFID] BŁĄD: Nie znaleziono modułu PN532!
```

**Rozwiązanie:**
1. Sprawdź połączenia I2C (Wire1: GPIO 33/32)
2. Upewnij się, że PN532 jest w trybie I2C
3. Sprawdź zasilanie (3.3V)
4. Spróbuj innego modułu PN532

### Brak dźwięku

**Rozwiązanie:**
1. Sprawdź połączenia I2S
2. Upewnij się, że DAC jest prawidłowo zasilany
3. Sprawdź czy plik MP3 jest prawidłowy
4. Zwiększ głośność przyciskami VOL+
5. Testuj z prostym plikiem MP3 (44.1kHz, stereo, 128kbps)

### Nie mogę połączyć z WiFi

**Reset WiFi:**
1. Naciśnij i przytrzymaj **VOL-** i **VOL+**
2. Naciśnij **RESET** na ESP32
3. Usłyszysz sygnał (jeśli podłączony buzzer)
4. ESP32 zrestartuje się w trybie AP

## 📚 Następne kroki

### Przypisz więcej kart

1. **Katalog:**
   - Typ: `dir`
   - Ścieżka: `/muzyka`

2. **Karty sterujące:**
   - **Repeat 3x:** typ `ctrl`, akcja `REPEAT3`
   - **Shuffle:** typ `ctrl`, akcja `SHUFFLE_ON`
   - **Sleep Timer:** typ `ctrl`, akcja `SLEEP_TIMER`, wartość `30` (minut)

### Połącz z siecią domową

1. W przeglądarce przejdź do **"Konfiguracja WiFi"**
2. Wpisz nazwę sieci (SSID)
3. Wpisz hasło
4. Kliknij **"Zapisz i restart"**
5. Po restarcie ESP32 połączy się z siecią
6. Znajdź nowy adres IP w Serial Monitor lub na ekranie OLED

### Upload plików przez FTP

```bash
# FileZilla lub inny klient FTP
Host: <IP ESP32>
Port: 21
User: bajko
Pass: grajek
```

### OTA Update

1. Przejdź do `http://<IP>/ota`
2. Wybierz plik `.bin` (skompilowany firmware)
3. Kliknij **"Wyślij firmware"**
4. Poczekaj na update i restart

## 🎵 Fizyczna playlista

**Jak to działa:**
1. Przyłóż **do 10 kart** jednocześnie na czytnik
2. Każda karta dodaje się do kolejki
3. Odtwarzanie rozpoczyna się automatycznie
4. Odbierz kartę - zostanie usunięta z kolejki
5. Odbierz wszystkie karty - odtwarzanie się zatrzyma

**Przykład:**
- Karta 1: bajka "Czerwony Kapturek"
- Karta 2: bajka "Trzy Świnki"
- Karta sterująca: REPEAT_INF

ESP32 będzie odtwarzać obie bajki w kółko, dopóki nie odbierzesz karty REPEAT!

## 💡 Porady

1. **Nazwy plików:** Używaj ASCII, unikaj polskich znaków
2. **Format MP3:** 44.1kHz, stereo, 128-320kbps
3. **Rozmiar SD:** Max 32GB dla FAT32
4. **Zasilanie:** ESP32 + peryferia = min 1A przy 5V
5. **Zasięg RFID:** ~3-5cm od czytnika
6. **Backup:** Regularnie kopiuj config.csv z karty SD

## 📞 Pomoc

- **GitHub Issues:** [BajkoGrajek Issues](https://github.com/tisher2/BajkoGrajek-RFID-MP3-Player/issues)
- **Serial Monitor:** Wszystkie logi w języku polskim
- **OLED:** Status i błędy na wyświetlaczu
- **Web UI:** Status systemu w czasie rzeczywistym

---

**Enjoy! 🎵**

*Szybki start zaktualizowany: 2025*
