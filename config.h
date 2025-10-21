#ifndef CONFIG_H
#define CONFIG_H

// ===== PINY ESP32 WR-32 =====
// Karta SD (SPI)
#define SD_SCK 18
#define SD_MOSI 23
#define SD_MISO 19
#define SD_CS 5

// Wzmacniacz Audio MAX98357A (I2S)
#define I2S_DOUT 22
#define I2S_BCLK 25
#define I2S_LRC 26

// Wyświetlacz OLED SSD1306 (I2C 0)
#define OLED_SDA 21
#define OLED_SCL 16
#define OLED_WIDTH 128
#define OLED_HEIGHT 32
#define OLED_ADDRESS 0x3C

// Czytnik RFID PN532 (I2C 1)
#define PN532_SDA 32
#define PN532_SCL 33
#define PN532_ADDRESS 0x24

// Przyciski (INPUT_PULLUP)
#define BTN_VOL_DOWN 17  // Przycisk 1: Ciszej -10%
#define BTN_PAUSE 13     // Przycisk 2: Pauza/Stop
#define BTN_PLAY 4       // Przycisk 3: Play
#define BTN_VOL_UP 27    // Przycisk 4: Głośniej +10%

// ===== KONFIGURACJA SYSTEMU =====
#define CONFIG_FILE "/config.csv"
#define SETTINGS_FILE "/settings.ini"
#define UPLOAD_TMP_EXT ".upload_tmp"

#define MAX_QUEUE_SIZE 10
#define RFID_DEBOUNCE_MS 1000
#define RFID_SCAN_TIMEOUT 30000
#define BUTTON_DEBOUNCE_MS 50

#define DEFAULT_VOLUME 25
#define VOLUME_STEP 10

#define AP_SSID "BajkoGrajek"
#define AP_PASSWORD ""

#define FTP_USER "bajko"
#define FTP_PASS "grajek"
#define FTP_PORT 21

#define WEB_PORT 80
#define MAX_UPLOAD_SIZE (100 * 1024 * 1024) // 100MB

// ===== TYPY KART RFID =====
enum CardType {
    CARD_NONE,
    CARD_FILE,
    CARD_DIR,
    CARD_CTRL
};

enum CtrlAction {
    CTRL_NONE,
    CTRL_REPEAT3,
    CTRL_REPEAT5,
    CTRL_REPEAT_INF,
    CTRL_SHUFFLE_ON,
    CTRL_SLEEP_TIMER
};

enum PlayerState {
    STATE_STOPPED,
    STATE_PLAYING,
    STATE_PAUSED
};

// ===== STRUKTURY DANYCH =====
struct RFIDCard {
    uint8_t uid[7];
    uint8_t uidLength;
    CardType type;
    String paths;
    CtrlAction action;
    uint32_t lastSeen;
    bool present;
};

struct QueueItem {
    RFIDCard card;
    int currentFileIndex;
    int repeatCount;
    bool shuffleMode;
};

struct AudioState {
    PlayerState state;
    String currentFile;
    uint32_t totalTime;
    uint32_t currentTime;
    int volume;
    int queueSize;
    CtrlAction activeCtrl;
    int sleepTimerMinutes;
    uint32_t sleepTimerStart;
};

// ===== ZMIENNE GLOBALNE =====
extern AudioState audioState;
extern QueueItem playQueue[MAX_QUEUE_SIZE];
extern int queueHead;
extern int queueTail;
extern RFIDCard lastScannedCard;
extern bool scanModeActive;
extern uint32_t scanModeStartTime;

extern SemaphoreHandle_t queueMutex;
extern SemaphoreHandle_t audioMutex;
extern QueueHandle_t audioCommandQueue;
extern QueueHandle_t displayQueue;

// ===== KOMENDY AUDIO =====
enum AudioCommand {
    CMD_PLAY,
    CMD_PAUSE,
    CMD_STOP,
    CMD_RESUME,
    CMD_VOL_UP,
    CMD_VOL_DOWN,
    CMD_NEXT,
    CMD_PREV
};

#endif
