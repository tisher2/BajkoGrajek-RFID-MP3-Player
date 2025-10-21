#include "config.h"

// ============================================================================
// DEFINICJE ZMIENNYCH GLOBALNYCH
// ============================================================================

// Kolejka odtwarzania
QueueItem playQueue[MAX_QUEUE_SIZE];
int queueSize = 0;

// Stan audio
AudioState audioState;

// Kolejki FreeRTOS
QueueHandle_t audioCommandQueue = nullptr;

// Mutexy
SemaphoreHandle_t sdMutex = nullptr;
SemaphoreHandle_t displayMutex = nullptr;
SemaphoreHandle_t queueMutex = nullptr;
SemaphoreHandle_t logMutex = nullptr;
SemaphoreHandle_t scanMutex = nullptr;

// Logi dla OLED
std::vector<String> displayLogs;

// Tryb skanowania dla Web UI
bool webScanMode = false;
String lastScannedUID = "";
unsigned long webScanStartTime = 0;
