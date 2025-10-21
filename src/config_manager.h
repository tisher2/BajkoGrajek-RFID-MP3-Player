#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <vector>
#include "config.h"

namespace ConfigManager {
  // Inicjalizacja i ładowanie
  void load();
  void save();
  
  // Zarządzanie kartami
  void addCard(const RFIDCard& card);
  void removeCard(const String& uid);
  RFIDCard findCard(const String& uid);
  std::vector<RFIDCard> getCards();
  
  // Pomocnicze
  int getCardCount();
}

#endif // CONFIG_MANAGER_H
