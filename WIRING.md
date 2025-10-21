# BajkoGrajek - Schemat połączeń

## 📐 Pełny schemat połączeń

### ESP32 Pinout

```
                           ESP32 DevKit
                    ┌─────────────────────┐
                    │                     │
                3V3 │o                   o│ GND
                    │                     │
                 EN │o                   o│ GPIO23 (SD_MOSI)
                    │                     │
          GPIO36/VP │o                   o│ GPIO22 (I2S_LRC, OLED_SCL)
                    │                     │
          GPIO39/VN │o                   o│ GPIO1 (TX)
                    │                     │
             GPIO34 │o                   o│ GPIO3 (RX)
                    │                     │
             GPIO35 │o                   o│ GPIO21 (OLED_SDA)
                    │                     │
 RFID_SCL  GPIO32 o│o                   o│ GND
                    │                     │
 RFID_SDA  GPIO33 o│o                   o│ GPIO19 (SD_MISO)
                    │                     │
 I2S_DOUT  GPIO25 o│o                   o│ GPIO18 (SD_SCK)
                    │                     │
 I2S_BCLK  GPIO26 o│o                   o│ GPIO5  (SD_CS)
                    │                     │
  BTN_VOL+ GPIO27 o│o                   o│ GPIO17 (BTN_VOL-)
                    │                     │
             GPIO14 │o                   o│ GPIO16
                    │                     │
             GPIO12 │o                   o│ GPIO4  (BTN_PLAY)
                    │                     │
                GND │o                   o│ GPIO0
                    │                     │
 BTN_PAUSE GPIO13 o│o                   o│ GPIO2
                    │                     │
                    │      ┌────┐        │
                    │      │USB │        │
                    │      └────┘        │
                    └─────────────────────┘
```

## 🔌 Szczegółowe połączenia

### 1. Karta SD (SPI)

**Moduł SD:**
```
SD Module          ESP32
────────────────────────
VCC         ───────  3.3V
GND         ───────  GND
CS          ───────  GPIO 5
MOSI        ───────  GPIO 23
SCK         ───────  GPIO 18
MISO        ───────  GPIO 19
```

**Uwagi:**
- Używaj **3.3V** zasilania
- Dodaj kondensator 10µF między VCC i GND
- Kabel max 15cm dla stabilności SPI
- Karta musi być FAT32

### 2. I2S DAC (Audio)

**Opcja A: MAX98357 I2S Amplifier**
```
MAX98357           ESP32
────────────────────────
VDD         ───────  5V (lub 3.3V)
GND         ───────  GND
DIN         ───────  GPIO 25 (I2S_DOUT)
BCLK        ───────  GPIO 26 (I2S_BCLK)
LRC         ───────  GPIO 22 (I2S_LRC)
SD          ───────  (nie podłączaj = max gain)
GAIN        ───────  (nie podłączaj = 15dB gain)
```

**Opcja B: PCM5102 DAC**
```
PCM5102            ESP32
────────────────────────
VCC         ───────  3.3V
GND         ───────  GND
BCK         ───────  GPIO 26 (I2S_BCLK)
DIN         ───────  GPIO 25 (I2S_DOUT)
LCK         ───────  GPIO 22 (I2S_LRC)
SCK         ───────  GND (bypass internal PLL)
FLT         ───────  GND (normal filter)
FMT         ───────  GND (I2S format)
```

**Podłączenie głośnika:**
- MAX98357: bezpośrednio 4-8Ω głośnik (bez wzmacniacza)
- PCM5102: przez wzmacniacz audio (line out)

### 3. OLED Display (I2C)

**SSD1306 OLED 128x32:**
```
OLED               ESP32
────────────────────────
VCC         ───────  3.3V
GND         ───────  GND
SCL         ───────  GPIO 22 (I2C0_SCL)
SDA         ───────  GPIO 21 (I2C0_SDA)
```

**Uwagi:**
- Adres I2C: 0x3C (domyślny)
- Rezystory pull-up 4.7kΩ (często wbudowane w moduł)
- Jeśli nie działa, sprawdź adres: 0x3C lub 0x3D

### 4. PN532 RFID Reader (I2C)

**PN532 Module:**
```
PN532              ESP32
────────────────────────
VCC         ───────  3.3V (lub 5V - sprawdź moduł)
GND         ───────  GND
SDA         ───────  GPIO 33 (I2C1_SDA)
SCL         ───────  GPIO 32 (I2C1_SCL)
```

**Konfiguracja PN532:**
- Ustaw przełączniki na tryb **I2C**:
  - SEL0: OFF
  - SEL1: ON
- Adres I2C: 0x24

**Uwagi:**
- Używamy Wire1 (drugi kanał I2C)
- Zasięg czytania: 3-5cm
- Kompatybilne karty: Mifare Classic, Mifare Ultralight, NFC

### 5. Przyciski

**Schemat jednego przycisku:**
```
                     ESP32 GPIO
                          │
                          │  (INPUT_PULLUP)
                          │
                    ┌─────┴─────┐
                    │  10kΩ     │
                    │  (wewn.)  │
                    └─────┬─────┘
                          │
                     ┌────┴────┐
                     │ Button  │
                     └────┬────┘
                          │
                         GND
```

**Wszystkie przyciski:**
```
Przycisk           ESP32          Funkcja
──────────────────────────────────────────
VOL DOWN    ───────  GPIO 17    Głośność -10%
PAUSE       ───────  GPIO 13    Pauza/Stop
PLAY        ───────  GPIO 4     Play/Next
VOL UP      ───────  GPIO 27    Głośność +10%
```

**Każdy przycisk:**
- Jeden koniec: GPIO
- Drugi koniec: GND
- Pull-up wewnętrzny (INPUT_PULLUP)
- Bez dodatkowych rezystorów

### 6. Buzzer (opcjonalny)

```
Buzzer             ESP32
────────────────────────
+           ───────  GPIO 15
-           ───────  GND
```

**Uwagi:**
- Używany tylko dla sygnału resetu WiFi
- Pasywny buzzer 3.3V-5V
- Częstotliwość sygnału: 5kHz przez 1s

## 🔧 Zasilanie

### Opcja A: USB (rozwojowa)
```
USB 5V ──── ESP32 (USB port)
         │
         └── Wszystkie peryferia zasilane z 3.3V ESP32
```

**Ograniczenia:**
- Max prąd z 3.3V pin: ~600mA
- Wystarcza dla małych głośników (MAX98357)

### Opcja B: Zewnętrzne zasilanie (produkcyjne)
```
5V 2A PSU ──┬── ESP32 (VIN)
            │
            └── MAX98357 (VDD)

LDO 3.3V 1A ──┬── SD Card
              ├── PN532
              ├── OLED
              └── ESP32 (3.3V backup)
```

**Zalecane:**
- Zasilacz 5V/2A
- Kondensatory: 100µF przy zasilaniu głównym, 10µF przy każdym module
- Osobne zasilanie dla ESP32 i MAX98357

### Pobór prądu (przybliżony)

| Komponent | Prąd | Uwagi |
|-----------|------|-------|
| ESP32 | 80-240mA | Zależnie od WiFi |
| SD Card | 50-200mA | Przy odczycie |
| PN532 | 50-150mA | Przy skanowaniu |
| OLED | 10-20mA | Przy max jasności |
| MAX98357 | 500mA-1.5A | Zależnie od głośności |
| PCM5102 | 50mA | Bez wzmacniacza |

**Całkowite zapotrzebowanie: ~1-2A przy 5V**

## ⚠️ Ważne uwagi

### Levels napięcia
- ESP32 GPIO: **3.3V**
- SD Card: **3.3V** (nigdy 5V!)
- PN532: Zazwyczaj **3.3V** lub 5V-tolerant (sprawdź moduł)
- OLED: **3.3V**
- MAX98357: **3.3V lub 5V**
- PCM5102: **3.3V**

### Pull-up rezystory
- I2C (OLED + RFID): 4.7kΩ (często wbudowane w moduły)
- Przyciski: INPUT_PULLUP wewnętrzny (10kΩ)

### Kondensatory filtrujące
- 10µF przy każdym module (VCC-GND)
- 100µF przy zasilaniu głównym
- 100nF ceramiczne blisko pinów VCC każdego IC

### Długość przewodów
- I2C: max 50cm (bez wzmacniaczy)
- SPI: max 15cm
- I2S: max 30cm
- Przyciski: max 1m

## 🧪 Testowanie połączeń

### Test 1: Zasilanie
```cpp
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 działa!");
}
```
Oczekiwany wynik: komunikat w Serial Monitor

### Test 2: SD Card
```cpp
#include <SD.h>
#include <SPI.h>

void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, 5);
  if (SD.begin(5)) {
    Serial.println("SD OK");
  } else {
    Serial.println("SD ERROR");
  }
}
```

### Test 3: I2C Scanner
```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  Wire.setClock(100000);
  
  Serial.println("Skanowanie I2C...");
  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Znaleziono: 0x");
      Serial.println(i, HEX);
    }
  }
}
```
Oczekiwany wynik: 0x3C (OLED)

### Test 4: PN532
```cpp
#include <Wire.h>
#include <Adafruit_PN532.h>

Adafruit_PN532 nfc(33, 32, &Wire);

void setup() {
  Serial.begin(115200);
  Wire.begin(33, 32);
  nfc.begin();
  
  uint32_t ver = nfc.getFirmwareVersion();
  if (ver) {
    Serial.print("PN532 OK: v");
    Serial.println((ver >> 24) & 0xFF, HEX);
  } else {
    Serial.println("PN532 ERROR");
  }
}
```

### Test 5: Przyciski
```cpp
void setup() {
  Serial.begin(115200);
  pinMode(17, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(27, INPUT_PULLUP);
}

void loop() {
  Serial.printf("BTN: %d %d %d %d\n",
    digitalRead(17), digitalRead(13),
    digitalRead(4), digitalRead(27));
  delay(500);
}
```
Oczekiwany wynik: 1 1 1 1 (wszystkie HIGH), 0 gdy wciśnięty

## 🔨 Lista zakupów

### Podstawowe komponenty
- ESP32 DevKit (~$5)
- Moduł SD Card Reader (~$1)
- MAX98357 I2S Amplifier (~$3) lub PCM5102 (~$2)
- PN532 NFC/RFID Reader (~$5)
- OLED SSD1306 128x32 I2C (~$3)
- 4x Tact Switch (~$1)
- Głośnik 4-8Ω (~$2)
- Karta microSD 8-32GB (~$5)
- Karty NFC Mifare (~$5 za 10 szt)

### Dodatkowe
- Buzzer pasywny (~$0.5)
- Zasilacz 5V/2A (~$3)
- Przewody jumper (~$2)
- Kondensatory (10µF, 100µF, 100nF) (~$2)
- Płytka prototypowa (~$2)

**Całkowity koszt: ~$35-45**

## 📷 Zdjęcia przykładowego montażu

*Tutaj można dodać zdjęcia rzeczywistego montażu*

---

*Schemat połączeń zaktualizowany: 2025*
