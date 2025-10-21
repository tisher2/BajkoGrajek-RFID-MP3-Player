#include "button_handler.h"

namespace ButtonHandler {
  // Handle task
  static TaskHandle_t buttonTaskHandle = nullptr;
  
  // Stany przycisków
  static bool btnStates[4] = {false, false, false, false};
  static unsigned long btnLastPress[4] = {0, 0, 0, 0};
  
  // Kliknięcie pauzy (1x = pauza, 2x = stop)
  static unsigned long pauseLastPress = 0;
  static int pauseClickCount = 0;
  
  // ========================================================================
  // INICJALIZACJA
  // ========================================================================
  
  void init() {
    Serial.println("[Button] Inicjalizacja modułu przycisków...");
    
    pinMode(BTN_VOL_DOWN, INPUT_PULLUP);
    pinMode(BTN_PAUSE, INPUT_PULLUP);
    pinMode(BTN_PLAY, INPUT_PULLUP);
    pinMode(BTN_VOL_UP, INPUT_PULLUP);
    
    Serial.println("[Button] Przyciski zainicjalizowane");
  }
  
  // ========================================================================
  // FREERTOS TASK
  // ========================================================================
  
  void createTask() {
    xTaskCreatePinnedToCore(
      buttonTask,
      "ButtonTask",
      2048,
      nullptr,
      3,  // Średni priorytet
      &buttonTaskHandle,
      0   // Core 0
    );
    Serial.println("[Button] Task utworzony na Core 0");
  }
  
  void buttonTask(void* parameter) {
    while (true) {
      // Sprawdź każdy przycisk
      bool volDown = digitalRead(BTN_VOL_DOWN) == LOW;
      bool pause = digitalRead(BTN_PAUSE) == LOW;
      bool play = digitalRead(BTN_PLAY) == LOW;
      bool volUp = digitalRead(BTN_VOL_UP) == LOW;
      
      unsigned long now = millis();
      
      // Volume Down
      if (volDown && !btnStates[0]) {
        if (now - btnLastPress[0] > BUTTON_DEBOUNCE_MS) {
          btnStates[0] = true;
          btnLastPress[0] = now;
          
          AudioCommand cmd = CMD_VOL_DOWN;
          xQueueSend(audioCommandQueue, &cmd, 0);
          Serial.println("[Button] VOL DOWN");
        }
      } else if (!volDown) {
        btnStates[0] = false;
      }
      
      // Pause (1x = pauza, 2x = stop)
      if (pause && !btnStates[1]) {
        if (now - btnLastPress[1] > BUTTON_DEBOUNCE_MS) {
          btnStates[1] = true;
          btnLastPress[1] = now;
          
          // Sprawdź czy to drugie kliknięcie
          if (now - pauseLastPress < 500) {  // 500ms na drugie kliknięcie
            // Drugie kliknięcie - STOP
            AudioCommand cmd = CMD_STOP;
            xQueueSend(audioCommandQueue, &cmd, 0);
            Serial.println("[Button] STOP (2x PAUSE)");
            pauseClickCount = 0;
          } else {
            // Pierwsze kliknięcie - PAUSE/RESUME
            if (audioState.state == PLAYER_PLAYING) {
              AudioCommand cmd = CMD_PAUSE;
              xQueueSend(audioCommandQueue, &cmd, 0);
              Serial.println("[Button] PAUSE");
            } else if (audioState.state == PLAYER_PAUSED) {
              AudioCommand cmd = CMD_RESUME;
              xQueueSend(audioCommandQueue, &cmd, 0);
              Serial.println("[Button] RESUME");
            }
            pauseClickCount = 1;
          }
          
          pauseLastPress = now;
        }
      } else if (!pause) {
        btnStates[1] = false;
      }
      
      // Play / Next
      if (play && !btnStates[2]) {
        if (now - btnLastPress[2] > BUTTON_DEBOUNCE_MS) {
          btnStates[2] = true;
          btnLastPress[2] = now;
          
          if (audioState.state == PLAYER_PLAYING) {
            // Jeśli już gra, przejdź do następnego
            AudioCommand cmd = CMD_NEXT;
            xQueueSend(audioCommandQueue, &cmd, 0);
            Serial.println("[Button] NEXT");
          } else {
            // Jeśli nie gra, rozpocznij odtwarzanie pierwszej karty z kolejki
            AudioCommand cmd = CMD_PLAY;
            xQueueSend(audioCommandQueue, &cmd, 0);
            Serial.println("[Button] PLAY");
          }
        }
      } else if (!play) {
        btnStates[2] = false;
      }
      
      // Volume Up
      if (volUp && !btnStates[3]) {
        if (now - btnLastPress[3] > BUTTON_DEBOUNCE_MS) {
          btnStates[3] = true;
          btnLastPress[3] = now;
          
          AudioCommand cmd = CMD_VOL_UP;
          xQueueSend(audioCommandQueue, &cmd, 0);
          Serial.println("[Button] VOL UP");
        }
      } else if (!volUp) {
        btnStates[3] = false;
      }
      
      vTaskDelay(pdMS_TO_TICKS(BUTTON_CHECK_MS));
    }
  }
  
  // ========================================================================
  // RESET WIFI
  // ========================================================================
  
  bool isResetWiFiPressed() {
    // Sprawdź czy przyciski 1 i 4 są wciśnięte (VOL_DOWN i VOL_UP)
    bool btn1 = digitalRead(BTN_VOL_DOWN) == LOW;
    bool btn4 = digitalRead(BTN_VOL_UP) == LOW;
    
    if (btn1 && btn4) {
      Serial.println("[Button] Wykryto kombinację resetu WiFi (przyciski 1+4)");
      return true;
    }
    
    return false;
  }
}
