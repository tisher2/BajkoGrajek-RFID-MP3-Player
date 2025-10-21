#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_PN532.h>
#include <Audio.h>
#include <esp_task_wdt.h>

#define SD_CS 5
#define I2S_DIN 22
#define I2S_BCLK 25
#define I2S_LRC 26
#define OLED_SDA 21
#define OLED_SCL 16
#define RFID_SDA 32
#define RFID_SCL 33

#define BUTTON_VOL_DOWN 17
#define BUTTON_STOP 13
#define BUTTON_PLAY 4
#define BUTTON_VOL_UP 27

Adafruit_SSD1306 display(OLED_SDA, OLED_SCL);
Adafruit_PN532 rfid(RFID_SDA, RFID_SCL);
AudioGeneratorMP3 mp3;
AudioFileSourceSD source;
AudioOutputI2S output;

TaskHandle_t audioTaskHandle;

void setup() {
    Serial.begin(115200);
    // Initialize SD card
    if (!SD.begin(SD_CS)) {
        Serial.println("SD Card initialization failed!");
        return;
    }
    // Initialize display
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    // Initialize RFID
    rfid.begin();
    // Initialize audio
    output.begin();
    // Button setup
    pinMode(BUTTON_VOL_DOWN, INPUT);
    pinMode(BUTTON_STOP, INPUT);
    pinMode(BUTTON_PLAY, INPUT);
    pinMode(BUTTON_VOL_UP, INPUT);
    // Create audio task
    xTaskCreatePinnedToCore(AudioTask, "AudioTask", 10000, NULL, 10, &audioTaskHandle, 1);
}

void loop() {
    // Handle button presses and RFID scanning
}

void AudioTask(void *pvParameters) {
    // Handle audio playback and decoding
}
