#include "WebServer.h"
#include "TimeManager.h"
#include "WebPage.h"
#include "Relay.h"

AsyncWebServer server(80);

Logger logger;
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
      // Mendapatkan nomor relay (misalnya relay1 atau relay2)
      byte relayNumber = (command == "relay1") ? 1 : (command == "relay2") ? 2
                                                                           : 0;

      if (relayNumber == 0)
      {
        json["error"] = "Invalid relay number.";
        Serial.println("Invalid relay number");
      }
      else
      {
        // Menangani perintah on, off, toggle, 0, 1, 2
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
          json["error"] = "Invalid command for relay " + String(relayNumber);
          Serial.println("Invalid command for relay " + String(relayNumber));
        }
      }
    }

    else
    {
      json["error"] = "Unknown or invalid command.";
      Serial.println("Unknown or Invalid command");
    }
    String response;
    serializeJson(json, response);
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
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

  server.onNotFound([](AsyncWebServerRequest *request)
                    {
    if (request->method() == HTTP_OPTIONS) {
        request->send(200);
    } else {
        // Check if the path is one of our frontend routes
        String path = request->url();
        if (path == "/" || path == "/cs" || path == "/st" || path == "/ad" || path == "/dp") {
            request->send_P(200, "text/html", htmlContent);
        } else {
            // Send 404 page
            request->send_P(404, "text/html", notFoundPage);
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

  server.on("/api/songs", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String json = storage.readJSON("/songs.json"); // Gunakan storage untuk memanggil readJSON
    request->send(200, "application/json", json); });

  server.on("/api/alarm", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String json = storage.readJSON("/alarm.json"); // Gunakan storage untuk memanggil readJSON
    request->send(200, "application/json", json); });

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
        int relayDuration = doc["relay_duration"].as<int>();
        
        // Add alarm
        alarmManager.addAlarm(label, time, days, ringtone, relay, relayDuration);
        
        // Send success response
        request->send(201, "application/json", "{\"message\":\"Alarm added successfully\"}");
        Serial.println("Alarm added successfully teko api"+body);

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
        int relayDuration = doc["relay_duration"].as<int>();

        // Try to update the alarm
        bool updated = alarmManager.updateAlarm(id, label, time, days, ringtone, relay, relayDuration);
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

  server.on("/api/relay", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String json = storage.readJSON("/relay.json"); // Gunakan storage untuk memanggil readJSON
    request->send(200, "application/json", json); });

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