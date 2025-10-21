#include "web_server.h"
#include "audio_handler.h"
#include "rfid_handler.h"
#include "config_manager.h"
#include "wifi_manager.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <SD.h>
#include <Update.h>

namespace WebServer {
  // Serwer HTTP
  static AsyncWebServer* server = nullptr;
  
  // HTML strony głównej
  static const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>BajkoGrajek</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: #fff;
      padding: 20px;
      min-height: 100vh;
    }
    .container {
      max-width: 800px;
      margin: 0 auto;
    }
    h1 {
      text-align: center;
      margin-bottom: 30px;
      font-size: 2.5em;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
    }
    .card {
      background: rgba(255,255,255,0.1);
      backdrop-filter: blur(10px);
      border-radius: 15px;
      padding: 20px;
      margin-bottom: 20px;
      box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37);
      border: 1px solid rgba(255,255,255,0.18);
    }
    .card h2 {
      margin-bottom: 15px;
      font-size: 1.5em;
    }
    .status-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
      gap: 10px;
      margin-bottom: 10px;
    }
    .status-item {
      background: rgba(255,255,255,0.15);
      padding: 10px;
      border-radius: 8px;
    }
    .status-label {
      font-size: 0.8em;
      opacity: 0.8;
    }
    .status-value {
      font-size: 1.2em;
      font-weight: bold;
      margin-top: 5px;
    }
    button {
      background: rgba(255,255,255,0.2);
      color: #fff;
      border: 1px solid rgba(255,255,255,0.3);
      padding: 10px 20px;
      border-radius: 8px;
      cursor: pointer;
      font-size: 1em;
      transition: all 0.3s;
    }
    button:hover {
      background: rgba(255,255,255,0.3);
      transform: translateY(-2px);
    }
    button:active {
      transform: translateY(0);
    }
    input, select {
      width: 100%;
      padding: 10px;
      margin: 5px 0;
      border-radius: 8px;
      border: 1px solid rgba(255,255,255,0.3);
      background: rgba(255,255,255,0.1);
      color: #fff;
      font-size: 1em;
    }
    input::placeholder {
      color: rgba(255,255,255,0.6);
    }
    .file-list {
      max-height: 300px;
      overflow-y: auto;
      background: rgba(0,0,0,0.2);
      border-radius: 8px;
      padding: 10px;
    }
    .file-item {
      padding: 10px;
      margin: 5px 0;
      background: rgba(255,255,255,0.1);
      border-radius: 8px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .file-item:hover {
      background: rgba(255,255,255,0.2);
    }
    .btn-group {
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
    }
    .progress {
      width: 100%;
      height: 20px;
      background: rgba(0,0,0,0.2);
      border-radius: 10px;
      overflow: hidden;
      margin: 10px 0;
    }
    .progress-bar {
      height: 100%;
      background: linear-gradient(90deg, #00d2ff 0%, #3a7bd5 100%);
      transition: width 0.3s;
    }
    .hidden { display: none; }
    @media (max-width: 600px) {
      h1 { font-size: 2em; }
      .status-grid { grid-template-columns: 1fr; }
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🎵 BajkoGrajek</h1>
    
    <!-- Status systemu -->
    <div class="card">
      <h2>Status Systemu</h2>
      <div class="status-grid">
        <div class="status-item">
          <div class="status-label">Stan</div>
          <div class="status-value" id="status-state">-</div>
        </div>
        <div class="status-item">
          <div class="status-label">Głośność</div>
          <div class="status-value" id="status-volume">-</div>
        </div>
        <div class="status-item">
          <div class="status-label">Kolejka</div>
          <div class="status-value" id="status-queue">-</div>
        </div>
        <div class="status-item">
          <div class="status-label">Karta SD</div>
          <div class="status-value" id="status-sd">-</div>
        </div>
      </div>
      <div id="status-file" style="margin-top: 10px; font-size: 0.9em; opacity: 0.8;">
        Brak odtwarzania
      </div>
    </div>
    
    <!-- Przeglądarka plików -->
    <div class="card">
      <h2>Pliki MP3</h2>
      <div style="margin-bottom: 10px;">
        <span id="current-path">/</span>
      </div>
      <div class="file-list" id="file-list">
        <div style="text-align: center; opacity: 0.6;">Ładowanie...</div>
      </div>
      <div class="btn-group" style="margin-top: 10px;">
        <button onclick="uploadFile()">📤 Wyślij plik MP3</button>
        <button onclick="refreshFiles()">🔄 Odśwież</button>
      </div>
    </div>
    
    <!-- Przypisywanie kart RFID -->
    <div class="card">
      <h2>Karty RFID</h2>
      <div>
        <button onclick="scanRFID()" id="scan-btn">📡 Skanuj kartę</button>
        <div id="scan-result" style="margin: 10px 0; min-height: 20px;"></div>
      </div>
      <div id="assign-form" class="hidden">
        <h3>Przypisz kartę: <span id="scanned-uid"></span></h3>
        <select id="card-type" onchange="updateAssignForm()">
          <option value="file">Pojedynczy plik</option>
          <option value="dir">Katalog</option>
          <option value="ctrl">Karta sterująca</option>
        </select>
        <div id="path-input">
          <input type="text" id="card-path" placeholder="Ścieżka do pliku/katalogu">
        </div>
        <div id="ctrl-input" class="hidden">
          <select id="ctrl-action">
            <option value="REPEAT3">Powtórz 3 razy</option>
            <option value="REPEAT5">Powtórz 5 razy</option>
            <option value="REPEAT_INF">Powtórz w nieskończoność</option>
            <option value="SHUFFLE_ON">Shuffle ON</option>
            <option value="SLEEP_TIMER">Sleep Timer</option>
          </select>
          <input type="number" id="timer-value" placeholder="Minuty" class="hidden">
        </div>
        <div class="btn-group" style="margin-top: 10px;">
          <button onclick="assignCard()">✅ Przypisz</button>
          <button onclick="cancelAssign()">❌ Anuluj</button>
        </div>
      </div>
      <div style="margin-top: 15px;">
        <label>
          <input type="checkbox" id="no-confirm" onchange="saveNoConfirm()">
          Nie pytaj o potwierdzenia
        </label>
      </div>
    </div>
    
    <!-- Konfiguracja WiFi -->
    <div class="card">
      <h2>Konfiguracja WiFi</h2>
      <input type="text" id="wifi-ssid" placeholder="Nazwa sieci (SSID)">
      <input type="password" id="wifi-pass" placeholder="Hasło">
      <button onclick="saveWiFi()" style="margin-top: 10px;">💾 Zapisz i restart</button>
    </div>
    
    <!-- Informacje -->
    <div class="card">
      <h2>Informacje</h2>
      <div style="line-height: 1.8;">
        <strong>IP:</strong> <span id="info-ip">-</span><br>
        <strong>FTP:</strong> ftp://bajko:grajek@<span id="info-ftp">-</span>:21<br>
        <strong>OTA Update:</strong> <a href="/ota" style="color: #fff;">Aktualizacja firmware</a>
      </div>
    </div>
  </div>

  <input type="file" id="file-upload" accept=".mp3" style="display: none;" onchange="handleFileUpload()">

  <script>
    let currentPath = '/';
    let scannedUID = '';
    
    // Załaduj preferencje
    if (localStorage.getItem('noConfirm') === 'true') {
      document.getElementById('no-confirm').checked = true;
    }
    
    function saveNoConfirm() {
      localStorage.setItem('noConfirm', document.getElementById('no-confirm').checked);
    }
    
    // Aktualizacja statusu
    function updateStatus() {
      fetch('/api/status')
        .then(r => r.json())
        .then(data => {
          document.getElementById('status-state').textContent = data.state;
          document.getElementById('status-volume').textContent = data.volume + '%';
          document.getElementById('status-queue').textContent = data.queue;
          document.getElementById('status-sd').textContent = data.sd ? 'OK' : 'BŁĄD';
          document.getElementById('status-file').textContent = data.file || 'Brak odtwarzania';
          document.getElementById('info-ip').textContent = data.ip;
          document.getElementById('info-ftp').textContent = data.ip;
        });
    }
    
    // Pliki
    function refreshFiles() {
      fetch('/api/files?path=' + encodeURIComponent(currentPath))
        .then(r => r.json())
        .then(data => {
          const list = document.getElementById('file-list');
          list.innerHTML = '';
          
          // Przycisk do góry
          if (currentPath !== '/') {
            const up = document.createElement('div');
            up.className = 'file-item';
            up.innerHTML = '<span>📁 ..</span>';
            up.onclick = () => {
              const parts = currentPath.split('/').filter(p => p);
              parts.pop();
              currentPath = '/' + parts.join('/');
              if (currentPath !== '/' && !currentPath.endsWith('/')) currentPath += '/';
              if (currentPath === '//') currentPath = '/';
              document.getElementById('current-path').textContent = currentPath;
              refreshFiles();
            };
            list.appendChild(up);
          }
          
          // Katalogi
          data.dirs.forEach(dir => {
            const item = document.createElement('div');
            item.className = 'file-item';
            item.innerHTML = '<span>📁 ' + dir + '</span>';
            item.onclick = () => {
              currentPath = currentPath + dir + '/';
              if (currentPath.startsWith('//')) currentPath = currentPath.substring(1);
              document.getElementById('current-path').textContent = currentPath;
              refreshFiles();
            };
            list.appendChild(item);
          });
          
          // Pliki
          data.files.forEach(file => {
            const item = document.createElement('div');
            item.className = 'file-item';
            item.innerHTML = `
              <span>🎵 ${file}</span>
              <div>
                <button onclick="playFile('${currentPath + file}'); event.stopPropagation();">▶️</button>
                <button onclick="deleteFile('${currentPath + file}'); event.stopPropagation();">🗑️</button>
              </div>
            `;
            list.appendChild(item);
          });
        });
    }
    
    function playFile(path) {
      if (!confirm('Odtworzyć: ' + path + '?')) return;
      fetch('/api/play', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({path})
      }).then(() => updateStatus());
    }
    
    function deleteFile(path) {
      if (!confirm('Usunąć plik: ' + path + '?')) return;
      fetch('/api/delete', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({path})
      }).then(() => refreshFiles());
    }
    
    function uploadFile() {
      document.getElementById('file-upload').click();
    }
    
    function handleFileUpload() {
      const file = document.getElementById('file-upload').files[0];
      if (!file) return;
      
      const formData = new FormData();
      formData.append('file', file);
      formData.append('path', currentPath);
      
      fetch('/api/upload', {
        method: 'POST',
        body: formData
      }).then(() => {
        alert('Plik wysłany!');
        refreshFiles();
      });
    }
    
    // RFID
    function scanRFID() {
      document.getElementById('scan-result').textContent = 'Skanowanie... Przyłóż kartę do czytnika.';
      document.getElementById('scan-btn').disabled = true;
      
      fetch('/api/scan-rfid', {method: 'POST'})
        .then(() => {
          // Sprawdzaj status
          const check = setInterval(() => {
            fetch('/api/scan-rfid-status')
              .then(r => r.json())
              .then(data => {
                if (data.uid) {
                  clearInterval(check);
                  scannedUID = data.uid;
                  document.getElementById('scan-result').textContent = 'Zeskanowano: ' + data.uid;
                  document.getElementById('scanned-uid').textContent = data.uid;
                  document.getElementById('assign-form').classList.remove('hidden');
                  document.getElementById('scan-btn').disabled = false;
                } else if (!data.scanning) {
                  clearInterval(check);
                  document.getElementById('scan-result').textContent = 'Timeout - spróbuj ponownie.';
                  document.getElementById('scan-btn').disabled = false;
                }
              });
          }, 500);
        });
    }
    
    function updateAssignForm() {
      const type = document.getElementById('card-type').value;
      document.getElementById('path-input').classList.toggle('hidden', type === 'ctrl');
      document.getElementById('ctrl-input').classList.toggle('hidden', type !== 'ctrl');
      
      const action = document.getElementById('ctrl-action').value;
      document.getElementById('timer-value').classList.toggle('hidden', action !== 'SLEEP_TIMER');
    }
    
    function assignCard() {
      const type = document.getElementById('card-type').value;
      let path = '';
      let action = '';
      let value = 0;
      
      if (type === 'ctrl') {
        action = document.getElementById('ctrl-action').value;
        if (action === 'SLEEP_TIMER') {
          value = parseInt(document.getElementById('timer-value').value) || 30;
          action += ':' + value;
        }
      } else {
        path = document.getElementById('card-path').value;
        if (!path) {
          alert('Podaj ścieżkę!');
          return;
        }
      }
      
      fetch('/api/assign-rfid', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({uid: scannedUID, type, path, action})
      }).then(() => {
        alert('Karta przypisana!');
        cancelAssign();
      });
    }
    
    function cancelAssign() {
      document.getElementById('assign-form').classList.add('hidden');
      document.getElementById('scan-result').textContent = '';
      scannedUID = '';
    }
    
    // WiFi
    function saveWiFi() {
      const ssid = document.getElementById('wifi-ssid').value;
      const pass = document.getElementById('wifi-pass').value;
      
      if (!ssid) {
        alert('Podaj nazwę sieci!');
        return;
      }
      
      if (!confirm('Zapisać WiFi i zrestartować urządzenie?')) return;
      
      fetch('/api/wifi', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({ssid, password: pass})
      }).then(() => {
        alert('Zapisano! Urządzenie się restartuje...');
      });
    }
    
    // Inicjalizacja
    updateStatus();
    refreshFiles();
    setInterval(updateStatus, 2000);
    
    // Event listenery dla ctrl-action
    document.getElementById('ctrl-action').addEventListener('change', function() {
      document.getElementById('timer-value').classList.toggle('hidden', this.value !== 'SLEEP_TIMER');
    });
  </script>
</body>
</html>
)rawliteral";

  // OTA HTML
  static const char OTA_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>OTA Update - BajkoGrajek</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: #fff;
      padding: 20px;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
    }
    .container {
      background: rgba(255,255,255,0.1);
      backdrop-filter: blur(10px);
      border-radius: 15px;
      padding: 40px;
      max-width: 500px;
      box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37);
    }
    h1 { text-align: center; margin-bottom: 30px; }
    input[type="file"] { margin: 20px 0; width: 100%; }
    button {
      background: rgba(255,255,255,0.2);
      color: #fff;
      border: 1px solid rgba(255,255,255,0.3);
      padding: 15px 30px;
      border-radius: 8px;
      cursor: pointer;
      font-size: 1.1em;
      width: 100%;
    }
    button:hover { background: rgba(255,255,255,0.3); }
    .progress {
      width: 100%;
      height: 30px;
      background: rgba(0,0,0,0.2);
      border-radius: 15px;
      overflow: hidden;
      margin: 20px 0;
    }
    .progress-bar {
      height: 100%;
      background: linear-gradient(90deg, #00d2ff 0%, #3a7bd5 100%);
      width: 0%;
      transition: width 0.3s;
    }
    #status { text-align: center; margin-top: 20px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔄 OTA Update</h1>
    <form method="POST" action="/update" enctype="multipart/form-data" id="upload-form">
      <input type="file" name="update" accept=".bin" required>
      <button type="submit">📤 Wyślij firmware</button>
    </form>
    <div class="progress">
      <div class="progress-bar" id="progress-bar"></div>
    </div>
    <div id="status"></div>
  </div>
  <script>
    document.getElementById('upload-form').addEventListener('submit', function(e) {
      e.preventDefault();
      const formData = new FormData(this);
      const xhr = new XMLHttpRequest();
      
      xhr.upload.addEventListener('progress', function(e) {
        if (e.lengthComputable) {
          const percent = (e.loaded / e.total) * 100;
          document.getElementById('progress-bar').style.width = percent + '%';
          document.getElementById('status').textContent = 'Wysyłanie: ' + Math.round(percent) + '%';
        }
      });
      
      xhr.addEventListener('load', function() {
        if (xhr.status === 200) {
          document.getElementById('status').textContent = '✅ Update zakończony! Urządzenie się restartuje...';
        } else {
          document.getElementById('status').textContent = '❌ Błąd: ' + xhr.responseText;
        }
      });
      
      xhr.open('POST', '/update');
      xhr.send(formData);
    });
  </script>
</body>
</html>
)rawliteral";
  
  // ========================================================================
  // INICJALIZACJA
  // ========================================================================
  
  void init() {
    Serial.println("[Web] Inicjalizacja serwera WWW...");
    
    server = new AsyncWebServer(WEB_SERVER_PORT);
    
    // Strona główna
    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
      request->send_P(200, "text/html", HTML_PAGE);
    });
    
    // Strona OTA
    server->on("/ota", HTTP_GET, [](AsyncWebServerRequest* request) {
      request->send_P(200, "text/html", OTA_PAGE);
    });
    
    // API: Status systemu
    server->on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
      request->send(200, "application/json", getStatusJSON());
    });
    
    // API: Lista plików
    server->on("/api/files", HTTP_GET, [](AsyncWebServerRequest* request) {
      String path = "/";
      if (request->hasParam("path")) {
        path = request->getParam("path")->value();
      }
      request->send(200, "application/json", getFilesJSON(path));
    });
    
    // API: Skanowanie RFID
    server->on("/api/scan-rfid", HTTP_POST, [](AsyncWebServerRequest* request) {
      RFIDHandler::startWebScan();
      request->send(200, "text/plain", "OK");
    });
    
    server->on("/api/scan-rfid-status", HTTP_GET, [](AsyncWebServerRequest* request) {
      String uid = RFIDHandler::getLastScanned();
      bool scanning = RFIDHandler::isWebScanning();
      
      String json = "{\"scanning\":";
      json += scanning ? "true" : "false";
      json += ",\"uid\":\"" + uid + "\"}";
      
      request->send(200, "application/json", json);
    });
    
    // API: Przypisanie karty RFID
    server->on("/api/assign-rfid", HTTP_POST, [](AsyncWebServerRequest* request) {}, nullptr,
      [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        StaticJsonDocument<512> doc;
        deserializeJson(doc, data);
        
        RFIDCard card;
        card.uid = doc["uid"].as<String>();
        card.uid.toUpperCase();
        
        String type = doc["type"].as<String>();
        if (type == "file") {
          card.type = CARD_FILE;
          card.path = doc["path"].as<String>();
        } else if (type == "dir") {
          card.type = CARD_DIR;
          card.path = doc["path"].as<String>();
        } else if (type == "ctrl") {
          card.type = CARD_CTRL;
          String action = doc["action"].as<String>();
          
          if (action.startsWith("SLEEP_TIMER:")) {
            card.action = CTRL_SLEEP_TIMER;
            int colonIdx = action.indexOf(':');
            card.value = action.substring(colonIdx + 1).toInt();
          } else {
            card.action = parseCtrlAction(action);
          }
        }
        
        ConfigManager::addCard(card);
        request->send(200, "text/plain", "OK");
      }
    );
    
    // API: Odtwarzanie pliku
    server->on("/api/play", HTTP_POST, [](AsyncWebServerRequest* request) {}, nullptr,
      [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        StaticJsonDocument<256> doc;
        deserializeJson(doc, data);
        
        String path = doc["path"].as<String>();
        AudioHandler::play(path);
        
        request->send(200, "text/plain", "OK");
      }
    );
    
    // API: Usuwanie pliku
    server->on("/api/delete", HTTP_POST, [](AsyncWebServerRequest* request) {}, nullptr,
      [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        StaticJsonDocument<256> doc;
        deserializeJson(doc, data);
        
        String path = doc["path"].as<String>();
        if (SD.remove(path)) {
          request->send(200, "text/plain", "OK");
        } else {
          request->send(500, "text/plain", "Error");
        }
      }
    );
    
    // API: Upload pliku
    server->on("/api/upload", HTTP_POST,
      [](AsyncWebServerRequest* request) {
        request->send(200, "text/plain", "OK");
      },
      [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
        static File uploadFile;
        
        if (index == 0) {
          String path = "/";
          if (request->hasParam("path", true)) {
            path = request->getParam("path", true)->value();
          }
          if (!path.endsWith("/")) path += "/";
          
          String uploadPath = path + filename + ".upload_tmp";
          uploadFile = SD.open(uploadPath, FILE_WRITE);
          Serial.printf("[Web] Upload rozpoczęty: %s\n", uploadPath.c_str());
        }
        
        if (uploadFile) {
          uploadFile.write(data, len);
        }
        
        if (final) {
          uploadFile.close();
          
          String path = "/";
          if (request->hasParam("path", true)) {
            path = request->getParam("path", true)->value();
          }
          if (!path.endsWith("/")) path += "/";
          
          String tempPath = path + filename + ".upload_tmp";
          String finalPath = path + filename;
          
          SD.remove(finalPath);
          SD.rename(tempPath, finalPath);
          
          Serial.printf("[Web] Upload zakończony: %s\n", finalPath.c_str());
        }
      }
    );
    
    // API: Konfiguracja WiFi
    server->on("/api/wifi", HTTP_POST, [](AsyncWebServerRequest* request) {}, nullptr,
      [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        StaticJsonDocument<256> doc;
        deserializeJson(doc, data);
        
        String ssid = doc["ssid"].as<String>();
        String password = doc["password"].as<String>();
        
        WiFiManager::saveCredentials(ssid, password);
        
        request->send(200, "text/plain", "OK");
        
        delay(1000);
        ESP.restart();
      }
    );
    
    // OTA Update
    server->on("/update", HTTP_POST,
      [](AsyncWebServerRequest* request) {
        bool success = !Update.hasError();
        request->send(200, "text/plain", success ? "OK" : "FAIL");
        delay(1000);
        ESP.restart();
      },
      [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
        if (index == 0) {
          Serial.printf("[Web] OTA Update rozpoczęty: %s\n", filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
          }
        }
        
        if (Update.write(data, len) != len) {
          Update.printError(Serial);
        }
        
        if (final) {
          if (Update.end(true)) {
            Serial.println("[Web] OTA Update zakończony pomyślnie");
          } else {
            Update.printError(Serial);
          }
        }
      }
    );
    
    server->begin();
    Serial.println("[Web] Serwer WWW uruchomiony na porcie 80");
    addDisplayLog("Web OK");
  }
  
  // ========================================================================
  // POMOCNICZE
  // ========================================================================
  
  String getStatusJSON() {
    StaticJsonDocument<512> doc;
    
    // Stan odtwarzacza
    switch (audioState.state) {
      case PLAYER_STOPPED:
        doc["state"] = "Zatrzymany";
        break;
      case PLAYER_PLAYING:
        doc["state"] = "Odtwarza";
        break;
      case PLAYER_PAUSED:
        doc["state"] = "Pauza";
        break;
    }
    
    doc["volume"] = audioState.volume;
    doc["queue"] = RFIDHandler::getQueueSize();
    doc["sd"] = SD.cardType() != CARD_NONE;
    doc["file"] = audioState.currentFile;
    doc["ip"] = WiFiManager::getIP();
    
    String output;
    serializeJson(doc, output);
    return output;
  }
  
  String getFilesJSON(const String& path) {
    StaticJsonDocument<2048> doc;
    JsonArray dirs = doc.createNestedArray("dirs");
    JsonArray files = doc.createNestedArray("files");
    
    File dir = SD.open(path);
    if (dir && dir.isDirectory()) {
      File file = dir.openNextFile();
      while (file) {
        String name = String(file.name());
        
        // Usuń ścieżkę z nazwy
        int lastSlash = name.lastIndexOf('/');
        if (lastSlash != -1) {
          name = name.substring(lastSlash + 1);
        }
        
        if (file.isDirectory()) {
          dirs.add(name);
        } else if (name.endsWith(".mp3") || name.endsWith(".MP3")) {
          files.add(name);
        }
        
        file.close();
        file = dir.openNextFile();
      }
      dir.close();
    }
    
    String output;
    serializeJson(doc, output);
    return output;
  }
}
