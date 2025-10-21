#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <vector>

// ============================================================================
// KONFIGURACJA PINÓW
// ============================================================================

// SD Card SPI (VSPI)
#define SD_CS       5
#define SD_SCK      18
#define SD_MISO     19
#define SD_MOSI     23

// I2S Audio
#define I2S_DOUT    25
#define I2S_BCLK    26
#define I2S_LRC     22

// I2C0 - OLED Display (Wire)
#define I2C0_SDA    21
#define I2C0_SCL    22

// I2C1 - PN532 RFID (Wire1)
#define I2C1_SDA    33
#define I2C1_SCL    32

// Przyciski
#define BTN_VOL_DOWN    17
#define BTN_PAUSE       13
#define BTN_PLAY        4
#define BTN_VOL_UP      27

// Buzzer (opcjonalny - dla sygnału resetu WiFi)
#define BUZZER_PIN      15

// ============================================================================
// STAŁE SYSTEMOWE
// ============================================================================

#define MAX_QUEUE_SIZE      10
#define BUTTON_DEBOUNCE_MS  50
#define RFID_DEBOUNCE_MS    1000
#define RFID_SCAN_INTERVAL_MS   200
#define DISPLAY_UPDATE_MS   100
#define BUTTON_CHECK_MS     20
#define WEB_SCAN_TIMEOUT_MS 30000

#define SPI_FREQUENCY       40000000  // 40 MHz dla SD
#define I2C_FREQUENCY       400000    // 400 kHz dla I2C

#define OLED_WIDTH          128
#define OLED_HEIGHT         32
#define OLED_ADDRESS        0x3C

#define DEFAULT_VOLUME      25        // Domyślna głośność 25%
#define VOLUME_STEP         10        // Krok zmiany głośności

// ============================================================================
// KONFIGURACJA WIFI I FTP
// ============================================================================

#define AP_SSID             "BajkoGrajek"
#define AP_PASSWORD         ""        // Bez hasła
#define AP_IP               "192.168.4.1"

#define FTP_USER            "bajko"
#define FTP_PASS            "grajek"
#define FTP_PORT            21

#define WEB_SERVER_PORT     80

// ============================================================================
// STRUKTURY DANYCH
// ============================================================================

// Typ karty RFID
enum CardType {
  CARD_FILE,      // Pojedynczy plik MP3
  CARD_DIR,       // Katalog z plikami MP3
  CARD_CTRL       // Karta sterująca
};

// Akcja sterująca
enum CtrlAction {
  CTRL_NONE,
  CTRL_REPEAT_3,
  CTRL_REPEAT_5,
  CTRL_REPEAT_INF,
  CTRL_SHUFFLE_ON,
  CTRL_SLEEP_TIMER
};

// Stan odtwarzacza
enum PlayerState {
  PLAYER_STOPPED,
  PLAYER_PLAYING,
  PLAYER_PAUSED
};

// Komenda audio
enum AudioCommand {
  CMD_NONE,
  CMD_PLAY,
  CMD_PAUSE,
  CMD_RESUME,
  CMD_STOP,
  CMD_NEXT,
  CMD_VOL_UP,
  CMD_VOL_DOWN
};

// Struktura karty RFID
struct RFIDCard {
  String uid;
  CardType type;
  String path;          // Ścieżka do pliku lub katalogu
  CtrlAction action;    // Akcja dla kart sterujących
  int value;            // Wartość dla SLEEP_TIMER (minuty)
  
  RFIDCard() : uid(""), type(CARD_FILE), path(""), action(CTRL_NONE), value(0) {}
};

// Element kolejki odtwarzania
struct QueueItem {
  RFIDCard card;
  bool active;
  unsigned long addedTime;
  
  QueueItem() : active(false), addedTime(0) {}
};

// Stan audio
struct AudioState {
  PlayerState state;
  int volume;
  String currentFile;
  unsigned long currentPosition;
  unsigned long totalDuration;
  bool repeat;
  int repeatCount;
  bool shuffle;
  unsigned long sleepTimer;
  
  AudioState() : state(PLAYER_STOPPED), volume(DEFAULT_VOLUME), 
                 currentFile(""), currentPosition(0), totalDuration(0),
                 repeat(false), repeatCount(0), shuffle(false), sleepTimer(0) {}
};

// ============================================================================
// ZMIENNE GLOBALNE (EXTERN)
// ============================================================================

// Kolejka odtwarzania
extern QueueItem playQueue[MAX_QUEUE_SIZE];
extern int queueSize;

// Stan audio
extern AudioState audioState;

// Kolejki FreeRTOS
extern QueueHandle_t audioCommandQueue;

// Mutexy
extern SemaphoreHandle_t sdMutex;
extern SemaphoreHandle_t displayMutex;
extern SemaphoreHandle_t queueMutex;

// Logi dla OLED
extern std::vector<String> displayLogs;
extern SemaphoreHandle_t logMutex;

// Tryb skanowania dla Web UI
extern bool webScanMode;
extern String lastScannedUID;
extern unsigned long webScanStartTime;
extern SemaphoreHandle_t scanMutex;

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

// Dodaj log do wyświetlania
inline void addDisplayLog(const String& msg) {
  extern SemaphoreHandle_t logMutex;
  extern std::vector<String> displayLogs;
  
  if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    displayLogs.push_back(msg);
    if (displayLogs.size() > 10) {  // Zachowaj tylko ostatnie 10 logów
      displayLogs.erase(displayLogs.begin());
    }
    xSemaphoreGive(logMutex);
  }
}

// Parsuj akcję sterującą ze stringa
inline CtrlAction parseCtrlAction(const String& actionStr) {
  if (actionStr == "REPEAT3") return CTRL_REPEAT_3;
  if (actionStr == "REPEAT5") return CTRL_REPEAT_5;
  if (actionStr == "REPEAT_INF") return CTRL_REPEAT_INF;
  if (actionStr == "SHUFFLE_ON") return CTRL_SHUFFLE_ON;
  if (actionStr.startsWith("SLEEP_TIMER")) return CTRL_SLEEP_TIMER;
  return CTRL_NONE;
}

// Konwertuj akcję sterującą na string
inline String ctrlActionToString(CtrlAction action, int value = 0) {
  switch (action) {
    case CTRL_REPEAT_3: return "REPEAT3";
    case CTRL_REPEAT_5: return "REPEAT5";
    case CTRL_REPEAT_INF: return "REPEAT_INF";
    case CTRL_SHUFFLE_ON: return "SHUFFLE_ON";
    case CTRL_SLEEP_TIMER: return "SLEEP_TIMER:" + String(value);
    default: return "NONE";
  }
}

#endif // CONFIG_H
