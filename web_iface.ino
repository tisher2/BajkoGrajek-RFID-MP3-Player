// web_iface.ino

#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESP32_FTPClient.h>

// FTP Server credentials
const char* ftpUser = "bajko";
const char* ftpPass = "grajek";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Function prototypes
void setupWebServer();
void setupFTPServer();

void setup() {
    // Initialize SPIFFS
    if (!SPIFFS.begin()) {
        Serial.println("Failed to mount file system");
        return;
    }
    WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...\n");
    }
    Serial.println("Connected to WiFi\n");
    setupWebServer();
    setupFTPServer();
}

void setupWebServer() {
    // Serve the main HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });

    // API endpoints
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Return system status
    });
    server.on("/api/files", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Return list of files
    });
    server.on("/api/scan-rfid", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Scan RFID
    });
    server.on("/api/assign-rfid", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Assign RFID
    });

    // Other endpoints...

    server.begin();
}

void setupFTPServer() {
    // Setup FTP Server
}

void loop() {
    // Main loop code
}
