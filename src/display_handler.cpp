#include "display_handler.h"
#include "rfid_handler.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace DisplayHandler {
  // Obiekt wyświetlacza
  static Adafruit_SSD1306* display = nullptr;
  
  // Handle task
  static TaskHandle_t displayTaskHandle = nullptr;
  
  // Scrolling logów
  static int logScrollOffset = 0;
  
  // ========================================================================
  // INICJALIZACJA
  // ========================================================================
  
  void init() {
    Serial.println("[Display] Inicjalizacja modułu wyświetlacza...");
    
    // Inicjalizacja I2C0 dla OLED
    Wire.begin(I2C0_SDA, I2C0_SCL);
    Wire.setClock(I2C_FREQUENCY);
    
    // Inicjalizacja wyświetlacza
    display = new Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
    
    if (!display->begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
      Serial.println("[Display] BŁĄD: Nie można zainicjalizować OLED!");
      return;
    }
    
    Serial.println("[Display] OLED zainicjalizowany");
    
    // Wyczyść ekran
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println("BajkoGrajek");
    display->println("Inicjalizacja...");
    display->display();
    
    // Inicjalizacja mutexa
    if (displayMutex == nullptr) {
      displayMutex = xSemaphoreCreateMutex();
    }
    
    if (logMutex == nullptr) {
      logMutex = xSemaphoreCreateMutex();
    }
  }
  
  // ========================================================================
  // FREERTOS TASK
  // ========================================================================
  
  void createTask() {
    xTaskCreatePinnedToCore(
      displayTask,
      "DisplayTask",
      4096,
      nullptr,
      2,  // Niski priorytet
      &displayTaskHandle,
      0   // Core 0
    );
    Serial.println("[Display] Task utworzony na Core 0");
  }
  
  void displayTask(void* parameter) {
    while (true) {
      update();
      vTaskDelay(pdMS_TO_TICKS(DISPLAY_UPDATE_MS));
    }
  }
  
  // ========================================================================
  // WYŚWIETLANIE
  // ========================================================================
  
  void update() {
    if (!display) return;
    
    if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      display->clearDisplay();
      display->setTextSize(1);
      display->setTextColor(SSD1306_WHITE);
      
      // Linia 1: Nazwa pliku (skrócona)
      display->setCursor(0, 0);
      if (audioState.currentFile.length() > 0) {
        // Wyodrębnij nazwę pliku
        int lastSlash = audioState.currentFile.lastIndexOf('/');
        String filename = audioState.currentFile.substring(lastSlash + 1);
        
        // Skróć jeśli za długa (max 21 znaków)
        if (filename.length() > 21) {
          filename = filename.substring(0, 18) + "...";
        }
        
        display->print(filename);
      } else {
        display->print("Brak odtwarzania");
      }
      
      // Linia 2: Głośność i kolejka
      display->setCursor(0, 10);
      display->print("Vol:");
      display->print(audioState.volume);
      display->print("% Q:");
      display->print(RFIDHandler::getQueueSize());
      
      // Status sterujący
      if (audioState.repeat) {
        if (audioState.repeatCount == 0) {
          display->print(" [R:INF]");
        } else {
          display->print(" [R:");
          display->print(audioState.repeatCount);
          display->print("]");
        }
      }
      if (audioState.shuffle) {
        display->print(" [S]");
      }
      if (audioState.sleepTimer > 0) {
        unsigned long remaining = (audioState.sleepTimer - millis()) / 60000;
        display->print(" [T:");
        display->print(remaining);
        display->print("m]");
      }
      
      // Linie 3-4: Logi (scrolling)
      if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        int logCount = displayLogs.size();
        if (logCount > 0) {
          // Wyświetl ostatnie 2 logi
          int startIdx = max(0, logCount - 2);
          int y = 20;
          
          for (int i = startIdx; i < logCount && y < 32; i++) {
            display->setCursor(0, y);
            String log = displayLogs[i];
            
            // Skróć log jeśli za długi
            if (log.length() > 21) {
              log = log.substring(0, 18) + "...";
            }
            
            display->print(log);
            y += 10;
          }
        }
        xSemaphoreGive(logMutex);
      }
      
      display->display();
      xSemaphoreGive(displayMutex);
    }
  }
  
  void clear() {
    if (!display) return;
    
    if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      display->clearDisplay();
      display->display();
      xSemaphoreGive(displayMutex);
    }
  }
  
  void showStatus() {
    update();
  }
  
  void showLog(const String& msg) {
    addDisplayLog(msg);
  }
}
