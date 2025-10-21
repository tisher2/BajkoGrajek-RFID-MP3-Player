#ifndef AUDIO_HANDLER_H
#define AUDIO_HANDLER_H

#include <Arduino.h>
#include "config.h"

namespace AudioHandler {
  // Inicjalizacja modułu audio
  void init();
  
  // Utworzenie FreeRTOS task
  void createTask();
  
  // Task audio (FreeRTOS)
  void audioTask(void* parameter);
  
  // Odtwarzanie
  void play(const String& path);
  void playNext();
  void stop();
  void pause();
  void resume();
  
  // Sterowanie głośnością
  void setVolume(int vol);
  void changeVolume(int delta);
  
  // Pętla główna (dekodowanie)
  void loop();
  
  // Status
  bool isPlaying();
  int getVolume();
  String getCurrentFile();
  
  // Obsługa kart sterujących
  void setRepeat(int count);
  void setShuffle(bool enabled);
  void setSleepTimer(unsigned long minutes);
}

#endif // AUDIO_HANDLER_H
