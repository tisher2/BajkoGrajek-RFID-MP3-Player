#include "audio_handler.h"
#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include <SD.h>
#include <Preferences.h>

namespace AudioHandler {
  // Obiekty audio
  static AudioGeneratorMP3* mp3 = nullptr;
  static AudioFileSourceSD* file = nullptr;
  static AudioFileSourceID3* id3 = nullptr;
  static AudioOutputI2S* out = nullptr;
  
  // Handle task
  static TaskHandle_t audioTaskHandle = nullptr;
  
  // Playlista dla katalogu
  static std::vector<String> playlist;
  static int playlistIndex = 0;
  
  // Preferences dla głośności
  static Preferences prefs;
  
  // ========================================================================
  // INICJALIZACJA
  // ========================================================================
  
  void init() {
    Serial.println("[Audio] Inicjalizacja modułu audio...");
    
    // Inicjalizacja SD
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, SPI, SPI_FREQUENCY)) {
      Serial.println("[Audio] BŁĄD: Nie można zainicjalizować karty SD!");
      addDisplayLog("SD BŁĄD!");
      return;
    }
    Serial.println("[Audio] Karta SD zainicjalizowana");
    addDisplayLog("SD OK");
    
    // Wczytaj głośność z pamięci
    prefs.begin("audio", false);
    audioState.volume = prefs.getInt("volume", DEFAULT_VOLUME);
    prefs.end();
    
    // Inicjalizacja I2S
    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    out->SetGain(audioState.volume / 100.0);
    
    Serial.printf("[Audio] Głośność: %d%%\n", audioState.volume);
    
    // Inicjalizacja kolejki komend
    audioCommandQueue = xQueueCreate(10, sizeof(AudioCommand));
  }
  
  // ========================================================================
  // FREERTOS TASK
  // ========================================================================
  
  void createTask() {
    xTaskCreatePinnedToCore(
      audioTask,
      "AudioTask",
      8192,
      nullptr,
      10,  // Wysoki priorytet
      &audioTaskHandle,
      1    // Core 1
    );
    Serial.println("[Audio] Task utworzony na Core 1");
  }
  
  void audioTask(void* parameter) {
    AudioCommand cmd;
    
    while (true) {
      // Sprawdź komendy
      if (xQueueReceive(audioCommandQueue, &cmd, 0) == pdTRUE) {
        switch (cmd) {
          case CMD_PAUSE:
            pause();
            break;
          case CMD_RESUME:
            resume();
            break;
          case CMD_STOP:
            stop();
            break;
          case CMD_NEXT:
            playNext();
            break;
          case CMD_VOL_UP:
            changeVolume(VOLUME_STEP);
            break;
          case CMD_VOL_DOWN:
            changeVolume(-VOLUME_STEP);
            break;
          default:
            break;
        }
      }
      
      // Dekodowanie audio
      if (mp3 && mp3->isRunning()) {
        if (!mp3->loop()) {
          // Zakończono odtwarzanie
          Serial.println("[Audio] Zakończono odtwarzanie pliku");
          
          // Sprawdź repeat
          if (audioState.repeat && audioState.repeatCount > 0) {
            audioState.repeatCount--;
            if (audioState.repeatCount > 0 || audioState.repeat) {
              // Odtwórz ponownie
              play(audioState.currentFile);
            } else {
              playNext();
            }
          } else {
            playNext();
          }
        }
      } else if (audioState.state == PLAYER_PLAYING) {
        // Jeśli powinno grać ale nie gra - błąd
        audioState.state = PLAYER_STOPPED;
      }
      
      // Sleep timer
      if (audioState.sleepTimer > 0) {
        if (millis() > audioState.sleepTimer) {
          Serial.println("[Audio] Sleep timer - zatrzymanie");
          stop();
          audioState.sleepTimer = 0;
        }
      }
      
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
  
  // ========================================================================
  // ODTWARZANIE
  // ========================================================================
  
  void play(const String& path) {
    // Zatrzymaj obecne odtwarzanie
    stop();
    
    Serial.printf("[Audio] Odtwarzanie: %s\n", path.c_str());
    
    // Sprawdź czy plik/katalog istnieje
    if (!SD.exists(path)) {
      Serial.println("[Audio] BŁĄD: Plik/katalog nie istnieje!");
      addDisplayLog("Plik nie istnieje!");
      return;
    }
    
    // Sprawdź czy to katalog
    File item = SD.open(path);
    if (item.isDirectory()) {
      Serial.println("[Audio] To katalog - tworzę playlistę");
      playlist.clear();
      playlistIndex = 0;
      
      // Odczytaj wszystkie pliki MP3 z katalogu
      File file = item.openNextFile();
      while (file) {
        String filename = String(file.name());
        if (filename.endsWith(".mp3") || filename.endsWith(".MP3")) {
          String fullPath = path;
          if (!fullPath.endsWith("/")) fullPath += "/";
          fullPath += filename;
          playlist.push_back(fullPath);
        }
        file.close();
        file = item.openNextFile();
      }
      item.close();
      
      Serial.printf("[Audio] Znaleziono %d plików MP3\n", playlist.size());
      
      if (playlist.empty()) {
        Serial.println("[Audio] BŁĄD: Brak plików MP3 w katalogu!");
        addDisplayLog("Brak MP3!");
        return;
      }
      
      // Shuffle jeśli włączony
      if (audioState.shuffle) {
        Serial.println("[Audio] Mieszanie playlisty");
        for (int i = playlist.size() - 1; i > 0; i--) {
          int j = random(0, i + 1);
          String temp = playlist[i];
          playlist[i] = playlist[j];
          playlist[j] = temp;
        }
      }
      
      // Odtwórz pierwszy plik
      play(playlist[0]);
      return;
    }
    item.close();
    
    // Odtwórz pojedynczy plik
    file = new AudioFileSourceSD(path.c_str());
    id3 = new AudioFileSourceID3(file);
    mp3 = new AudioGeneratorMP3();
    
    if (mp3->begin(id3, out)) {
      audioState.state = PLAYER_PLAYING;
      audioState.currentFile = path;
      Serial.println("[Audio] Rozpoczęto odtwarzanie");
      
      // Wyodrębnij nazwę pliku
      int lastSlash = path.lastIndexOf('/');
      String filename = path.substring(lastSlash + 1);
      addDisplayLog("Odtwarzam: " + filename);
    } else {
      Serial.println("[Audio] BŁĄD: Nie można rozpocząć odtwarzania!");
      addDisplayLog("BŁĄD audio!");
      delete mp3;
      delete id3;
      delete file;
      mp3 = nullptr;
      id3 = nullptr;
      file = nullptr;
    }
  }
  
  void playNext() {
    if (playlist.empty()) {
      Serial.println("[Audio] Brak kolejnego pliku");
      stop();
      return;
    }
    
    playlistIndex++;
    if (playlistIndex >= playlist.size()) {
      if (audioState.repeat) {
        playlistIndex = 0;
      } else {
        Serial.println("[Audio] Koniec playlisty");
        stop();
        return;
      }
    }
    
    play(playlist[playlistIndex]);
  }
  
  void stop() {
    if (mp3) {
      mp3->stop();
      delete mp3;
      mp3 = nullptr;
    }
    if (id3) {
      delete id3;
      id3 = nullptr;
    }
    if (file) {
      file->close();
      delete file;
      file = nullptr;
    }
    
    audioState.state = PLAYER_STOPPED;
    audioState.currentFile = "";
    playlist.clear();
    playlistIndex = 0;
    
    Serial.println("[Audio] Zatrzymano");
  }
  
  void pause() {
    if (audioState.state == PLAYER_PLAYING) {
      audioState.state = PLAYER_PAUSED;
      Serial.println("[Audio] Pauza");
      addDisplayLog("Pauza");
    }
  }
  
  void resume() {
    if (audioState.state == PLAYER_PAUSED) {
      audioState.state = PLAYER_PLAYING;
      Serial.println("[Audio] Wznowienie");
      addDisplayLog("Wznowiono");
    }
  }
  
  // ========================================================================
  // GŁOŚNOŚĆ
  // ========================================================================
  
  void setVolume(int vol) {
    if (vol < 0) vol = 0;
    if (vol > 100) vol = 100;
    
    audioState.volume = vol;
    if (out) {
      out->SetGain(vol / 100.0);
    }
    
    // Zapisz do pamięci
    prefs.begin("audio", false);
    prefs.putInt("volume", vol);
    prefs.end();
    
    Serial.printf("[Audio] Głośność: %d%%\n", vol);
    addDisplayLog("Vol: " + String(vol) + "%");
  }
  
  void changeVolume(int delta) {
    setVolume(audioState.volume + delta);
  }
  
  // ========================================================================
  // STATUS
  // ========================================================================
  
  void loop() {
    // Ta funkcja jest wywoływana przez task
  }
  
  bool isPlaying() {
    return audioState.state == PLAYER_PLAYING;
  }
  
  int getVolume() {
    return audioState.volume;
  }
  
  String getCurrentFile() {
    return audioState.currentFile;
  }
  
  // ========================================================================
  // KARTY STERUJĄCE
  // ========================================================================
  
  void setRepeat(int count) {
    if (count == 0) {
      audioState.repeat = true;
      audioState.repeatCount = 0;  // Nieskończone
      Serial.println("[Audio] Powtarzanie: NIESKOŃCZONE");
      addDisplayLog("Repeat: INF");
    } else {
      audioState.repeat = true;
      audioState.repeatCount = count;
      Serial.printf("[Audio] Powtarzanie: %d razy\n", count);
      addDisplayLog("Repeat: " + String(count) + "x");
    }
  }
  
  void setShuffle(bool enabled) {
    audioState.shuffle = enabled;
    Serial.printf("[Audio] Shuffle: %s\n", enabled ? "ON" : "OFF");
    addDisplayLog(enabled ? "Shuffle ON" : "Shuffle OFF");
  }
  
  void setSleepTimer(unsigned long minutes) {
    if (minutes > 0) {
      audioState.sleepTimer = millis() + (minutes * 60000);
      Serial.printf("[Audio] Sleep timer: %lu minut\n", minutes);
      addDisplayLog("Sleep: " + String(minutes) + "min");
    } else {
      audioState.sleepTimer = 0;
      Serial.println("[Audio] Sleep timer wyłączony");
    }
  }
}
