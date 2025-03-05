#include "WebServer.h"
#include "TimeManager.h"
#include "Relay.h"
#include "TimerManager.h"
#include "Audio.h"
#include "Buzzer.h"
#include "AlarmManager.h"

// #include "index_html_gz.h"
// #include "notfound_html.gz.h"

AsyncWebServer server(80);

Webserver webserver;

void Webserver::init()
{

  setupServerRoutes();
  server.begin();
}

// Fungsi untuk menangani perintah
void handleCommand(AsyncWebServerRequest *request)
{
  if (request->hasParam("cmd"))
  {
    String cmd = request->getParam("cmd")->value();
    cmd.trim();
    cmd.toLowerCase(); // Case-insensitive

    // Pecah perintah dan nilai
    int spaceIndex = cmd.indexOf(' ');
    String command = spaceIndex > 0 ? cmd.substring(0, spaceIndex) : cmd;
    String valueStr = spaceIndex > 0 ? cmd.substring(spaceIndex + 1) : "";
    int value = valueStr.toInt();

    // Hilangkan spasi di sekitar valueStr
    valueStr.trim();

    JsonDocument json;

    if (command == "volume" && !valueStr.isEmpty())
    {
      if (value >= 0 && value <= 30)
      {
        audio.volume(value);
        storage.updateJSON("/audio.json", "volume", String(value));
        json["volume"] = value;
        Serial.println("Volume set to " + String(value));
      }
      else
      {
        json["error"] = "Volume must be between 0 and 30.";
        Serial.println("Volume must be between 0 and 30");
      }
    }
    else if (command == "play" && !valueStr.isEmpty())
    {
      if (value > 0)
      {
        audio.play(value);
        json["play"] = value;
        Serial.println("Playing track " + String(value));
      }
      else
      {
        json["error"] = "Invalid track number.";
        Serial.println("Invalid Track Number");
      }
    }
    else if (command == "eq" && !valueStr.isEmpty())
    {
      if (value >= 0 && value <= 5)
      {
        audio.eq(value);
        storage.updateJSON("/audio.json", "equalizer", String(value));
        json["eq"] = value;
        Serial.println("Set EQ to " + String(value));
      }
      else
      {
        json["error"] = "EQ value must be between -1 and 5.";
        Serial.println("EQ value must be 0-5");
      }
    }
    else if (command == "source" && !valueStr.isEmpty())
    {
      if (value >= 0 && value <= 5)
      {
        audio.playbackSource(value);
        json["source"] = value;
        Serial.println("Set audio source to " + String(value));
      }
      else
      {
        json["error"] = "Invalid source value.";
        Serial.println("Invalid audio source");
      }
    }
    else if (command == "pause")
    {
      audio.pause();
      json["pause"] = true;
      Serial.println("Audio Paused");
    }
    else if (command == "resume")
    {
      audio.resume();
      json["resume"] = true;
      Serial.println("Audio resumed");
    }
    else if (command == "next")
    {
      audio.playNext();
      json["next"] = true;
      Serial.println("Audio next");
    }
    else if (command == "prev")
    {
      audio.playPrevious();
      json["prev"] = true;
      Serial.println("Audio Previous");
    }
    else if (command == "stop")
    {
      audio.stop();
      json["stop"] = true;
      Serial.println("Audio Stopped");
    }
    else if (command == "playad" && !valueStr.isEmpty())
    {
      if (value > 0)
      {
        audio.playAdvertisement(value);
        json["playad"] = value;
        Serial.println("Playing advertisement " + String(value));
      }
      else
      {
        json["error"] = "Invalid advertisement track number.";
        Serial.println("Invalid advertisement track number");
      }
    }
    else if (command == "stopad")
    {
      audio.stopAdvertisement();
      json["stopad"] = true;
      Serial.println("Stopped advertisement");
    }
    else if (command == "incvolume")
    {
      audio.incVolume();
      json["incvolume"] = true;
      Serial.println("Volume increased");
    }
    else if (command == "decvolume")
    {
      audio.decVolume();
      json["decvolume"] = true;
      Serial.println("Volume decreased");
    }
    else if (command == "restart")
    {
      json["restart"] = true;
      request->send(200, "application/json", "{\"restart\":true}");
      Serial.println("Restarting ESP");
      ESP.restart();
      return;
    }
    else if (command == "buzzer" && !valueStr.isEmpty())
    {
      if (value > 0 && value <= 10)
      {
        buzzer.beep(value);
        json["beep"] = value;
        Serial.println("Buzzer beep " + String(value));
      }
      else
      {
        json["error"] = "Invalid buzzer value";
        Serial.println("Invalid buzzer value");
      }
    }
    else if (command.startsWith("relay") && !valueStr.isEmpty())
    {
      // Mendapatkan nomor relay (relay1, relay2, relay3, relay4 atau semua relay jika relay0)
      byte relayNumber = 0;

      if (command == "relay1")
        relayNumber = 1;
      else if (command == "relay2")
        relayNumber = 2;
      else if (command == "relay3")
        relayNumber = 3;
      else if (command == "relay4")
        relayNumber = 4;

      if (relayNumber == 0 && (valueStr == "on" || valueStr == "1"))
      {
        // Menghidupkan semua relay
        for (byte i = 1; i <= 4; i++)
        {
          relay.on(i);
          json["relay" + String(i)] = "on";
        }
        Serial.println("All relays turned ON");
      }
      else if (relayNumber == 0 && (valueStr == "off" || valueStr == "0"))
      {
        // Mematikan semua relay
        for (byte i = 1; i <= 4; i++)
        {
          relay.off(i);
          json["relay" + String(i)] = "off";
        }
        Serial.println("All relays turned OFF");
      }
      else if (relayNumber == 0 && (valueStr == "toggle" || valueStr == "2"))
      {
        // Toggle semua relay
        for (byte i = 1; i <= 4; i++)
        {
          relay.toggle(i);
          json["relay" + String(i)] = "toggled";
        }
        Serial.println("All relays toggled");
      }
      else if (relayNumber == 0)
      {
        json["error"] = "Invalid command for all relays.";
        Serial.println("Invalid command for all relays");
      }
      else
      {
        // Menangani perintah untuk relay individu
        if (valueStr == "on" || valueStr == "1")
        {
          // Menghidupkan relay
          relay.on(relayNumber);
          json["relay" + String(relayNumber)] = "on";
          Serial.println("Relay " + String(relayNumber) + " turned ON");
        }
        else if (valueStr == "off" || valueStr == "0")
        {
          // Mematikan relay
          relay.off(relayNumber);
          json["relay" + String(relayNumber)] = "off";
          Serial.println("Relay " + String(relayNumber) + " turned OFF");
        }
        else if (valueStr == "toggle" || valueStr == "2")
        {
          // Toggle relay (ubah status ke kebalikannya)
          relay.toggle(relayNumber);
          json["relay" + String(relayNumber)] = "toggled";
          Serial.println("Relay " + String(relayNumber) + " toggled");
        }
        else
        {
          // Try to parse the value as a duration
          unsigned long duration = valueStr.toInt();
          if (duration > 0)
          {
            // Turn on relay for specified duration
            relay.onWithDuration(relayNumber, duration);
            json["relay" + String(relayNumber)] = "on_duration";
            json["duration"] = duration;
            Serial.println("Relay " + String(relayNumber) + " turned ON for " + String(duration) + "ms");
          }
          else
          {
            json["error"] = "Invalid command for relay " + String(relayNumber);
            Serial.println("Invalid command for relay " + String(relayNumber));
          }
        }
      }
    }
    else if (command == "timerstart")
    {
      json["timerstart"] = true;
      request->send(200, "application/json", "{\"timerstart\":true}");
      Serial.println("starting timer");
      timerManager.start();
      return;
    }
    else if (command == "timerstop")
    {
      json["timerstop"] = true;
      request->send(200, "application/json", "{\"timerstop\":true}");
      Serial.println("timer stopped");
      timerManager.stop();
      return;
    }
    else if (command == "time")
    {
      String json = timeManager.getTimeJson();
      request->send(200, "application/json", json);
      return;
    }
    else if (command == "timer")
    {
      String json = storage.readJSON("/timer.json"); // Gunakan storage untuk memanggil readJSON
      request->send(200, "application/json", json);
      return;
    }
    else if (command == "alarm")
    {
      String json = storage.readJSON("/alarm.json"); // Gunakan storage untuk memanggil readJSON
      request->send(200, "application/json", json);
      return;
    }
    else if (command == "info")
    {
      uint32_t freeHeap = ESP.getFreeHeap();
      String response = "{\"free_heap\":" + String(freeHeap) + "}";
      request->send(200, "application/json", response);
      return;
    }

    else
    {
      json["error"] = "Unknown or invalid command.";
      Serial.println("Unknown or Invalid command");
    }
    String response;
    serializeJsonPretty(json, response);
    request->send(200, "application/json", response);
  }
  else
  {
    request->send(400, "application/json", "{\"error\":\"Missing 'cmd' parameter.\"}");
  }
}

void Webserver::setupServerRoutes()
{
  DefaultHeaders::Instance().addHeader("Permissions-Policy", "interest-cohort=(), browsing-topics=(), private-state-token-redemption=(), private-state-token-issuance=()");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS, PUT, DELETE");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

  // server.onNotFound([](AsyncWebServerRequest *request)
  //                   {
  //   if (request->method() == HTTP_OPTIONS) {
  //       request->send(200);
  //   } else {
  //       String path = request->url();

  //       // Cek apakah path termasuk salah satu route yang diperbolehkan
  //       if (path == "/" || path == "/cs" || path == "/st" || path == "/ad" || path == "/tm") {
  //           AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, index_html_gz_len);
  //           response->addHeader("Content-Encoding", "gzip");
  //           request->send(response);
  //       } else {
  //           // Jika path tidak ditemukan, kirim halaman 404 dalam format gzip
  //           AsyncWebServerResponse *response = request->beginResponse_P(404, "text/html", notFound_html_gz, notFound_html_gz_len);
  //           response->addHeader("Content-Encoding", "gzip");
  //           request->send(response);
  //       }
  //   } });

  //============== ORI RAKENEK ============================
  server.onNotFound([this](AsyncWebServerRequest *request)
                    {
    if (request->method() == HTTP_OPTIONS) {
        request->send(200);
    } else {
        String path = request->url();
        String filePath = (path == "/" || path == "/cs" || path == "/st" || path == "/ad" || path == "/tm") 
                          ? "/index.html.gz" 
                          : "/404.html.gz";
        
        // Pastikan hanya ada SATU Content-Disposition
        if (!storage.serveFile(filePath, request, "text/html", "gzip")) {
            request->send(404, "text/plain", "File not found");
        }
    } });

  // Command handler
  server.on("/cm", HTTP_GET, handleCommand);

  server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String json = timeManager.getTimeJson();
    request->send(200, "application/json", json); });

  server.on("/api/audio", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String json = storage.readJSON("/audio.json"); // Gunakan storage untuk memanggil readJSON
    request->send(200, "application/json", json); });

  server.on("/api/songs", HTTP_GET, [this](AsyncWebServerRequest *request)
            {
      // Gunakan handler khusus untuk file JSON yang besar
      this->handleChunkedJsonRequest(request, "/songs.json"); });

  server.on("/api/timer", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String json = storage.readJSON("/timer.json"); // Gunakan storage untuk memanggil readJSON
    request->send(200, "application/json", json); });

  server.on("/api/timer", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
    static String body = "";
    body += String((char *)data).substring(0, len);

    if (index + len == total) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, body);
        
        if (error) {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }
        
        // Tambahkan validasi untuk id
        if (!doc["id"].is<int>()) {
            request->send(400, "application/json", "{\"error\":\"Timer ID is required\"}");
            return;
        }

        int id = doc["id"] | 0;
        unsigned long onTime = doc["on_time"] | 0;
        unsigned long offTime = doc["off_time"] | 0;
        int cycleCount = doc["cycle"] | 0;
        bool reversed = doc["reversed"] | false;

        // Validasi ID
        if (id < 1 || id > 4) {
            request->send(400, "application/json", "{\"error\":\"Invalid timer ID (must be 1-4)\"}");
            return;
        }
        
        bool success = timerManager.addTimer(id, onTime, offTime, cycleCount, reversed);
        
        if (success) {
            String response = "{\"message\":\"Timer added successfully to relay " + String(id) + "\"}";
            request->send(201, "application/json", response);
        } else {
            String response = "{\"error\":\"Timer ID " + String(id) + " is already in use\"}";
            request->send(400, "application/json", response);
        }

        body = "";
    } });

  // PUT - Update existing timer
  server.on("/api/timer", HTTP_PUT, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
    static String body = "";
    body += String((char *)data).substring(0, len);

    if (index + len == total) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, body);
        
        if (error) {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }
        
        if (!doc["id"].is<int>()) {
            request->send(400, "application/json", "{\"error\":\"Timer ID is required\"}");
            return;
        }
        
        int id = doc["id"];
        unsigned long onTime = doc["on_time"] | 0;
        unsigned long offTime = doc["off_time"] | 0;
        int cycleCount = doc["cycle"] | 0;
        bool reversed = doc["reversed"] | false;
        
        bool success = timerManager.updateTimer(id, onTime, offTime, cycleCount, reversed);
        
        if (success) {
            request->send(200, "application/json", "{\"message\":\"Timer updated successfully\"}");
        } else {
            request->send(404, "application/json", "{\"error\":\"Timer not found\"}");
        }

        body = "";
    } });

  // DELETE - Remove timer
  server.on("/api/timer", HTTP_DELETE, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("id")) {
        int id = request->getParam("id")->value().toInt();
        bool success = timerManager.deleteTimer(id);
        
        if (success) {
            request->send(200, "application/json", "{\"message\":\"Timer deleted successfully\"}");
        } else {
            request->send(404, "application/json", "{\"error\":\"Timer not found\"}");
        }
    } else {
        request->send(400, "application/json", "{\"error\":\"Timer ID is required\"}");
    } });

  server.on("/api/timerstate", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String json = storage.readJSON("/timerstate.json"); // Gunakan storage untuk memanggil readJSON
    request->send(200, "application/json", json); });

  server.on("/api/alarm", HTTP_GET, [this](AsyncWebServerRequest *request)
            {
// Gunakan handler khusus untuk file JSON yang besar
this->handleChunkedJsonRequest(request, "/alarm.json"); });

  // POST Alarm - Add a new alarm
  server.on("/api/alarm", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
    static String body = ""; // Static variable to accumulate data
    body += String((char *)data).substring(0, len);

    // If all data has been received
    if (index + len == total) {
        // Parse JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, body);
        
        if (error) {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }
        
        // Extract alarm details
        String label = doc["label"].as<String>();
        String time = doc["time"].as<String>();
        String days = doc["days"].as<String>();
        int ringtone = doc["ringtone"].as<int>();
        String relay = doc["relay"].as<String>();
        
        // Add alarm
        alarmManager.addAlarm(label, time, days, ringtone, relay);
        
        // Send success response
        request->send(201, "application/json", "{\"message\":\"Alarm added successfully\"}");
        Serial.println("Alarm added successfully teko api"+body);

        // Reset body for next request
        body = "";
    } });

  server.on("/api/sensor", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // Simulasi data sensor
    int temperature = 31;
    int humidity = 64;

    // Buat JSON response
    DynamicJsonDocument doc(256);
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;

    // Konversi JSON ke string
    String jsonResponse;
    serializeJsonPretty(doc, jsonResponse);

    // Kirim response
    request->send(200, "application/json", jsonResponse); });

  //==================TESTING==================
  server.on("/api/writealarm", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
    static String body = ""; // Static variable to accumulate data
    body += String((char *)data).substring(0, len);

    // If all data has been received
    if (index + len == total) {
        // Write the raw data directly to alarm.json
        storage.writeJSON("/alarm.json", body); // Write raw data to file

        // Send response
        String response = "{\"message\":\"Alarms updated successfully\"}";
        request->send(200, "application/json", response);

        // Reset body for next request
        body = "";
    } });

  // UPDATE ALRM PUT
  server.on("/api/alarm", HTTP_PUT, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
    static String body = ""; // Static variable to accumulate data
    body += String((char *)data).substring(0, len);

    // If all data has been received
    if (index + len == total) {
        // Parse JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, body);
        
        if (error) {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }
        
        // Extract the ID and new details from the JSON body
        if (!doc["id"].is<int>()) {
            request->send(400, "application/json", "{\"error\":\"ID is required\"}");
            return;
        }
        
        int id = doc["id"].as<int>();
        String label = doc["label"].as<String>();
        String time = doc["time"].as<String>();
        String days = doc["days"].as<String>();
        int ringtone = doc["ringtone"].as<int>();
        String relay = doc["relay"].as<String>();

        // Try to update the alarm
        bool updated = alarmManager.updateAlarm(id, label, time, days, ringtone, relay);
        if (updated) {
            request->send(200, "application/json", "{\"message\":\"Alarm updated successfully\"}");
        } else {
            request->send(404, "application/json", "{\"error\":\"Alarm not found\"}");
        }

        // Reset body for next request
        body = "";
    } });

  // DELETE Alarm - Remove an alarm by ID
  server.on("/api/alarm", HTTP_DELETE, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("id")) {
        int id = request->getParam("id")->value().toInt();
        alarmManager.removeAlarm(id);
        String response = "{\"message\":\"Alarm " + String(id) + " deleted successfully\"}";
        request->send(200, "application/json", response);
    } else {
        request->send(400, "application/json", "{\"error\":\"No alarm ID provided\"}");
    } });

  server.on("/api/writeaudio", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
              static String body = ""; // Variabel statis untuk mengumpulkan data
              body += String((char *)data).substring(0, len); // Menambahkan data body

              // Jika semua data sudah diterima (index + len == total)
              if (index + len == total) {
                  // Tulis data JSON ke file songs.json
                  storage.writeJSON("/audio.json", body);

                  // Kirim respons sukses
                  request->send(200, "application/json", "{\"status\":\"success\"}");

                  // Reset body untuk request berikutnya
                  body = "";
              } });

  server.on("/api/writesongs", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
              static String body = ""; // Variabel statis untuk mengumpulkan data
              body += String((char *)data).substring(0, len); // Menambahkan data body

              // Jika semua data sudah diterima (index + len == total)
              if (index + len == total) {
                  // Tulis data JSON ke file songs.json
                  storage.writeJSON("/songs.json", body);

                  // Kirim respons sukses
                  request->send(200, "application/json", "{\"status\":\"success\"}");

                  // Reset body untuk request berikutnya
                  body = "";
              } });

  server.on("/api/writerelay", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
              static String body = ""; // Variabel statis untuk mengumpulkan data
              body += String((char *)data).substring(0, len); // Menambahkan data body

              // Jika semua data sudah diterima (index + len == total)
              if (index + len == total) {
                  // Tulis data JSON ke file songs.json
                  storage.writeJSON("/relay.json", body);

                  // Kirim respons sukses
                  request->send(200, "application/json", "{\"status\":\"success\"}");

                  // Reset body untuk request berikutnya
                  body = "";
              } });

  // GET endpoint to fetch all relay data
  server.on("/api/relay", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  String json = storage.readJSON("/relay.json");
  request->send(200, "application/json", json); });

  // PUT endpoint to update a specific relay
  server.on("/api/relay", HTTP_PUT, [](AsyncWebServerRequest *request)
            {
    // Check if the body is available
    if (request->contentLength() > 0) {
      request->send(500, "application/json", "{\"error\":\"Body parsing not implemented\"}");
      return;
    }
  
    String body = request->arg("plain");
    
    // Parse the incoming JSON
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
      // JSON parsing failed
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    // Read existing relay configuration
    String currentJson = storage.readJSON("/relay.json");
    DynamicJsonDocument currentDoc(1024);
    deserializeJson(currentDoc, currentJson);
    
    // Update the specific relay
    for (JsonPair relay : doc.as<JsonObject>()) {
      String relayKey = relay.key().c_str();
      
      // Update duration
      if (relay.value().containsKey("duration")) {
        currentDoc[relayKey]["duration"] = relay.value()["duration"];
      }
      
      // Optionally update state if provided
      if (relay.value().containsKey("state")) {
        currentDoc[relayKey]["state"] = relay.value()["state"];
      }
    }
    
    // Convert back to JSON string
    String updatedJson;
    serializeJsonPretty(currentDoc, updatedJson);
    
    // Save updated configuration
    storage.writeJSON("/relay.json", updatedJson);
    
    // Send back the updated configuration
    request->send(200, "application/json", updatedJson); });

  server.on("/api/info", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t chipId = ESP.getChipId();
    uint32_t flashChipId = ESP.getFlashChipId();
    uint32_t flashChipSize = ESP.getFlashChipSize();
    uint32_t programSize = ESP.getSketchSize();
    uint32_t freeProgramSpace = ESP.getFreeSketchSpace();
    
    
    String firmwareVersion = "1.0.3"; // Ganti dengan versi firmware Anda
    String coreVersion = ESP.getCoreVersion();
    String sdkVersion = ESP.getSdkVersion();
    unsigned long uptime = millis() / 1000; // Uptime dalam detik
    String restartReason = ESP.getResetReason();
    
    String ssid = WiFi.SSID();
    int32_t rssi = WiFi.RSSI();
    String wifiModeStr;
    WiFiMode_t wifiMode = WiFi.getMode();
    switch (wifiMode) {
        case WIFI_OFF: wifiModeStr = "OFF"; break;
        case WIFI_STA: wifiModeStr = "STA"; break;
        case WIFI_AP: wifiModeStr = "AP"; break;
        case WIFI_AP_STA: wifiModeStr = "AP+STA"; break;
        default: wifiModeStr = "UNKNOWN";
    }
    
    int channel = WiFi.channel();
    String mac = WiFi.macAddress();
    String ip = WiFi.localIP().toString();
    String gateway = WiFi.gatewayIP().toString();
    
    String response = "{"
        "\"firmware_version\":\"" + firmwareVersion + "\","
        "\"core_version\":\"" + coreVersion + "\","
        "\"sdk_version\":\"" + sdkVersion + "\","
        "\"uptime\":" + String(uptime) + ","
        "\"restart_reason\":\"" + restartReason + "\","
        "\"ssid\":\"" + ssid + "\","
        "\"rssi\":" + String(rssi) + ","
        "\"wifi_mode\":\"" + wifiModeStr + "\","
        "\"channel\":" + String(channel) + ","
        "\"mac\":\"" + mac + "\","
        "\"ip\":\"" + ip + "\","
        "\"gateway\":\"" + gateway + "\","
        "\"chip_id\":\"" + String(chipId, HEX) + "\","
        "\"flash_chip_id\":\"" + String(flashChipId, HEX) + "\","
        "\"flash_size\":" + String(flashChipSize) + ","
        "\"program_size\":" + String(programSize) + ","
        "\"free_program_space\":" + String(freeProgramSpace) + ","
        "\"free_heap\":" + String(freeHeap) + ","
        "\"filesystem_size\":" + "1024000" + ""
    "}";
    
    request->send(200, "application/json", response); });

  //============================== FILE MANAGER ====================================

  // Endpoint untuk membaca isi file
  server.on("/api/files/read", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  if (!request->hasParam("path")) {
      request->send(400, "application/json", "{\"error\":\"Missing path parameter\"}");
      return;
  }

  String path = request->getParam("path")->value();
  if (!path.startsWith("/")) {
      path = "/" + path;
  }
  
  if (!LittleFS.exists(path)) {
      request->send(404, "application/json", "{\"error\":\"File not found\"}");
      return;
  }
  
  File file = LittleFS.open(path, "r");
  if (!file) {
      request->send(500, "application/json", "{\"error\":\"Failed to open file\"}");
      return;
  }

  // Deteksi jenis konten berdasarkan ekstensi file
  String contentType = "text/plain";
  if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  else if (path.endsWith(".json")) contentType = "application/json";
  
  // Untuk file kecil, baca langsung
  if (file.size() < 4096) {
      String content = "";
      while (file.available()) {
          content += (char)file.read();
      }
      file.close();
      request->send(200, contentType, content);
  } else {
      // Untuk file besar, gunakan streaming response
      AsyncResponseStream *response = request->beginResponseStream(contentType);
      
      // Buffer untuk membaca file secara bertahap
      const size_t bufferSize = 1024;
      uint8_t buffer[bufferSize];
      size_t bytesRead;
      
      while ((bytesRead = file.read(buffer, bufferSize)) > 0) {
          response->write(buffer, bytesRead);
      }
      
      file.close();
      request->send(response);
  } });

  server.on("/api/files/write", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              request->send(200); // Kirim respon awal
            },
            NULL, // Handler untuk data upload via `upload` (tidak diperlukan di sini)
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
        // Pastikan parameter `path` ada di query string
        if (!request->hasParam("path")) {
            request->send(400, "application/json", "{\"error\":\"Missing path parameter\"}");
            return;
        }

        // Ambil path dari query parameter
        String path = request->getParam("path")->value();
        if (!path.startsWith("/")) {
            path = "/" + path;
        }

        // Jika index == 0, berarti ini adalah awal request (chunk pertama)
        if (index == 0) {
            Serial.printf("Opening file: %s\n", path.c_str());
            File file = LittleFS.open(path, "w");
            if (!file) {
                request->send(500, "application/json", "{\"error\":\"Failed to open file for writing\"}");
                return;
            }
            file.close();
        }

        // Buka file dalam mode "append" untuk menulis potongan data
        File file = LittleFS.open(path, "a");
        if (!file) {
            request->send(500, "application/json", "{\"error\":\"Failed to open file for appending\"}");
            return;
        }

        // Tulis data dari body request
        file.write(data, len);
        file.close();

        // Jika total sama dengan (index + len), berarti ini adalah akhir request (chunk terakhir)
        if (index + len == total) {
            request->send(200, "application/json", "{\"success\":true,\"message\":\"File updated successfully\"}");
        } });

  // Endpoint untuk mendapatkan daftar semua file
  server.on("/api/files", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  JsonDocument doc;
  JsonArray fileList = doc.to<JsonArray>();
  
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
      JsonObject fileObj = fileList.createNestedObject();
      fileObj["name"] = dir.fileName();
      fileObj["size"] = dir.fileSize();
      fileObj["type"] = "file"; // ESP8266 LittleFS tidak memiliki konsep direktori
  }
  
  String json;
  serializeJsonPretty(doc, json);
  request->send(200, "application/json", json); });

  // Endpoint untuk membuat file baru
  server.on("/api/files/create", HTTP_POST, [](AsyncWebServerRequest *request)
            {
  if (!request->hasParam("path", true)) {
      request->send(400, "application/json", "{\"error\":\"Missing path parameter\"}");
      return;
  }
  
  String path = request->getParam("path", true)->value();
  if (!path.startsWith("/")) {
      path = "/" + path;
  }
  
  // Cek apakah file sudah ada
  if (LittleFS.exists(path)) {
      request->send(409, "application/json", "{\"error\":\"File already exists\"}");
      return;
  }
  
  // Membuat file kosong
  File file = LittleFS.open(path, "w");
  if (!file) {
      request->send(500, "application/json", "{\"error\":\"Failed to create file\"}");
      return;
  }
  
  // Jika ada konten awal, tulis ke file
  if (request->hasParam("content", true)) {
      String content = request->getParam("content", true)->value();
      file.print(content);
  }
  
  file.close();
  request->send(201, "application/json", "{\"success\":true,\"message\":\"File created successfully\"}"); });

  // Endpoint untuk menghapus file
  server.on("/api/files/delete", HTTP_DELETE, [](AsyncWebServerRequest *request)
            {
  if (!request->hasParam("path")) {
      request->send(400, "application/json", "{\"error\":\"Missing path parameter\"}");
      return;
  }
  
  String path = request->getParam("path")->value();
  if (!path.startsWith("/")) {
      path = "/" + path;
  }
  
  if (!LittleFS.exists(path)) {
      request->send(404, "application/json", "{\"error\":\"File not found\"}");
      return;
  }
  
  if (LittleFS.remove(path)) {
      request->send(200, "application/json", "{\"success\":true,\"message\":\"File deleted successfully\"}");
  } else {
      request->send(500, "application/json", "{\"error\":\"Failed to delete file\"}");
  } });

  // Endpoint untuk rename file
  server.on("/api/files/rename", HTTP_POST, [](AsyncWebServerRequest *request)
            {
  if (!request->hasParam("oldPath", true) || !request->hasParam("newPath", true)) {
      request->send(400, "application/json", "{\"error\":\"Missing oldPath or newPath parameter\"}");
      return;
  }
  
  String oldPath = request->getParam("oldPath", true)->value();
  String newPath = request->getParam("newPath", true)->value();
  
  if (!oldPath.startsWith("/")) {
      oldPath = "/" + oldPath;
  }
  
  if (!newPath.startsWith("/")) {
      newPath = "/" + newPath;
  }
  
  if (!LittleFS.exists(oldPath)) {
      request->send(404, "application/json", "{\"error\":\"Source file not found\"}");
      return;
  }
  
  if (LittleFS.exists(newPath)) {
      request->send(409, "application/json", "{\"error\":\"Destination file already exists\"}");
      return;
  }
  
  // LittleFS tidak memiliki fungsi rename yang langsung, jadi kita gunakan strategi copy-delete
  File sourceFile = LittleFS.open(oldPath, "r");
  File destFile = LittleFS.open(newPath, "w");
  
  if (!sourceFile || !destFile) {
      request->send(500, "application/json", "{\"error\":\"Failed to open source or destination file\"}");
      return;
  }
  
  // Salin konten
  while (sourceFile.available()) {
      destFile.write(sourceFile.read());
  }
  
  sourceFile.close();
  destFile.close();
  
  // Hapus file asli
  if (LittleFS.remove(oldPath)) {
      request->send(200, "application/json", "{\"success\":true,\"message\":\"File renamed successfully\"}");
  } else {
      // Jika gagal menghapus file asli, hapus file tujuan juga
      LittleFS.remove(newPath);
      request->send(500, "application/json", "{\"error\":\"Failed to complete rename operation\"}");
  } });

  // server.on("/api/deletejson", HTTP_POST, [](AsyncWebServerRequest *request)
  //           {
  //   if (request->hasParam("filename") && request->hasParam("key")) {
  //       String filename = request->getParam("filename")->value();
  //       String key = request->getParam("key")->value();

  //       if (!filename.startsWith("/")) {
  //           filename = "/" + filename;
  //       }

  //       // Baca konten file JSON
  //       String jsonContent = storage.readJSON(filename);
  //       if (jsonContent.isEmpty()) {
  //           request->send(404, "application/json", "{\"error\":\"File not found\"}");
  //           return;
  //       }

  //       // Parse JSON dan hapus key
  //       JsonDocument doc; // Ukuran buffer dapat disesuaikan
  //       DeserializationError error = deserializeJson(doc, jsonContent);
  //       if (error) {
  //           request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
  //           return;
  //       }

  //       // Hapus key jika ada
  //       if (doc.containsKey(key)) {
  //           doc.remove(key);
  //           String updatedJson;
  //           serializeJson(doc, updatedJson);

  //           // Tulis ulang file JSON
  //           storage.writeJSON(filename, updatedJson);
  //           request->send(200, "application/json", "{\"status\":\"success\"}");
  //       } else {
  //           request->send(404, "application/json", "{\"error\":\"Key not found\"}");
  //       }
  //   } else {
  //       request->send(400, "application/json", "{\"error\":\"Missing parameters: filename or key\"}");
  //   } });
}

void Webserver::handleChunkedJsonRequest(AsyncWebServerRequest *request, const String &filename) {
  struct ChunkedState {
      File file;
      bool isFirstChunk;
      bool isClosed;
  };

  // Alokasi state di heap
  auto *state = new ChunkedState();
  state->isFirstChunk = true;
  state->isClosed = false;

  // Cleanup saat request selesai atau terputus
  request->onDisconnect([state]() {
      if (!state->isClosed && state->file) {
          state->file.close();
          state->isClosed = true;
      }
      delete state;
  });

  // Handler untuk chunked response
  AsyncWebServerResponse *response = request->beginChunkedResponse(
      "application/json",
      [state, filename](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
          if (state->isFirstChunk) {
              state->file = LittleFS.open(filename, "r");
              state->isFirstChunk = false;
              if (!state->file) {
                  delete state;
                  return 0;
              }
          }

          if (!state->file || !state->file.available() || state->isClosed) {
              if (!state->isClosed && state->file) {
                  state->file.close();
                  state->isClosed = true;
              }
              return 0;
          }

          size_t bytesRead = state->file.read(buffer, maxLen);
          if (bytesRead == 0) {
              if (!state->isClosed) {
                  state->file.close();
                  state->isClosed = true;
              }
          }
          return bytesRead;
      }
  );

  request->send(response);
}