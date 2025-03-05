#include "Storage.h"

Storage storage;

#include <LittleFS.h>
#include "Storage.h"
// #include "WebServer.h"
#include "Config.h"

void Storage::init()
{
    // Coba mount LittleFS
    if (!LittleFS.begin())
    {
        Serial.println("Failed to mount LittleFS. Attempting to format...");

        // Format LittleFS jika gagal mount
        if (LittleFS.format())
        {
            Serial.println("LittleFS formatted successfully.");
        }
        else
        {
            Serial.println("Failed to format LittleFS. Aborting...");
            return;
        }
    }
    else
    {
        Serial.println("LittleFS mounted successfully.");
    }

    // Periksa dan buat file default jika belum ada
    if (!LittleFS.exists("/audio.json"))
    {
        int volume = DEFAULT_VOLUME; // Ambil nilai volume dari config.h
        int eq = DEFAULT_EQ;
        Serial.println("audio.json not found. Creating default...");
        String defaultAudio = "{\"volume\":" + String(volume) + ",\"equalizer\":" + String(eq) + "}";
        writeJSON("/audio.json", defaultAudio);
        Serial.println("Default audio.json created.");
    }
    if (!LittleFS.exists("/songs.json"))
    {
        Serial.println("songs.json not found. Creating default...");

        // Definisi default JSON
        String defaultSong = R"rawliteral([
        {
            "id": "1",
            "title": "Audio1",
            "duration": "0:08"
        },
        {
            "id": "2",
            "title": "Audio2",
            "duration": "0:29"
        },
        {
            "id": "3",
            "title": "Audio3",
            "duration": "0:30"
        },
        {
            "id": "4",
            "title": "Audio4",
            "duration": "0:30"
        },
        {
            "id": "5",
            "title": "Audio5",
            "duration": "0:29"
        }
    ])rawliteral"; // Menggunakan raw string literal agar lebih rapi

        writeJSON("/songs.json", defaultSong);
        Serial.println("Default songs.json created.");
    }

    if (!LittleFS.exists("/alarm.json"))
    {
        Serial.println("alarm.json not found. Creating default...");

        // JSON default untuk alarm
        String defaultAlarm = R"rawliteral([
        {
            "id": 1,
            "label": "Morning",
            "time": "07:00",
            "days": "0111111",
            "ringtone": 1,
            "relay": "1:0,0:0,1:3000,1:4000"
        },
        {
            "id": 2,
            "label": "Afternoon",
            "time": "12:00",
            "days": "1111111",
            "ringtone": 2,
            "relay": "0:0,0:0,0:0,0:0"
        },
        {
            "id": 3,
            "label": "Evening",
            "time": "18:00",
            "days": "1010101",
            "ringtone": 3,
            "relay": "1:0,1:0,1:0,1:0"
        },
        {
            "id": 4,
            "label": "Night",
            "time": "20:00",
            "days": "1111111",
            "ringtone": 4,
            "relay": "1:2000,1:0,1:0,1:4000"
        }
    ])rawliteral"; // Raw string literal untuk memudahkan format JSON

        writeJSON("/alarm.json", defaultAlarm);
        Serial.println("Default alarm.json created.");
    }

    if (!LittleFS.exists("/relay.json"))
    {
        Serial.println("relay.json not found. Creating default...");
        String defaultRelay = "{\"relay1\":{\"state\":\"off\",\"duration\":1000},\"relay2\":{\"state\":\"off\",\"duration\":2000},\"relay3\":{\"state\":\"off\",\"duration\":3000},\"relay4\":{\"state\":\"off\",\"duration\":4000}}";
        writeJSON("/relay.json", defaultRelay);
        Serial.println("Default relay.json created.");
    }
    if (!LittleFS.exists("/timer.json"))
    {
        Serial.println("timer.json not found. Creating default...");

        // JSON default untuk timer
        String defaultTimer = R"rawliteral([
        {
            "id": 1,
            "on_time": 1000,
            "off_time": 1000,
            "cycle": -1,
            "reversed": true
        },
        {
            "id": 2,
            "on_time": 2000,
            "off_time": 2000,
            "cycle": -1,
            "reversed": true
        },
        {
            "id": 3,
            "on_time": 3000,
            "off_time": 3000,
            "cycle": 3,
            "reversed": false
        },
        {
            "id": 4,
            "on_time": 4000,
            "off_time": 2000,
            "cycle": 5,
            "reversed": false
        }
    ])rawliteral"; // Raw string literal untuk format JSON yang lebih rapi

        writeJSON("/timer.json", defaultTimer);
        Serial.println("Default timer.json created.");
    }

    if (!LittleFS.exists("/timerstate.json"))
    {
        Serial.println("timerstate.json not found. Creating default...");

        // JSON default untuk timerstate
        String defaultTimerState = R"rawliteral({"isRunning": false})rawliteral";

        writeJSON("/timerstate.json", defaultTimerState);
        Serial.println("Default timerstate.json created.");
    }
}

String Storage::readJSON(const String &filename)
{
    if (!LittleFS.exists(filename))
    {
        Serial.println("File " + filename + " does not exist.");
        return "{}";
    }

    File file = LittleFS.open(filename, "r");
    if (!file)
    {
        Serial.println("Failed to open file " + filename + " for reading");
        return "{}";
    }

    // Untuk file kecil, tetap gunakan metode yang ada
    if (file.size() < 4096)
    { // Batas yang aman untuk ESP8266 (bisa disesuaikan)
        String jsonContent;
        while (file.available())
        {
            jsonContent += char(file.read());
        }
        file.close();
        return jsonContent;
    }
    else
    {
        // Untuk file besar, berikan peringatan dan kembalikan pesan error
        // Gunakan handleChunkedJsonRequest untuk file besar
        Serial.println("WARNING: File " + filename + " too large for direct reading. Use chunked response instead.");
        file.close();
        return "{\"error\":\"File too large for direct reading\"}";
    }
}

void Storage::writeJSON(const String &filename, const String &jsonContent)
{
    // Pastikan nama file memiliki awalan '/'
    String fullFilename = filename.startsWith("/") ? filename : "/" + filename;

    File file = LittleFS.open(fullFilename, "w");
    if (!file)
    {
        Serial.println("Failed to open file " + fullFilename + " for writing");
        return;
    }

    file.print(jsonContent);
    file.close();
}

// ONLY UPDATE VALUE ON SPECIFIC KEY
void Storage::updateJSON(const String &filename, const String &key, const String &newValue)
{
    // Membaca isi file JSON
    String jsonContent = readJSON(filename);
    if (jsonContent == "{}")
    {
        Serial.println("Error: JSON file is empty or not found.");
        return;
    }

    // Membuat objek JSON dari konten yang dibaca
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonContent);
    if (error)
    {
        Serial.println("Error: Failed to parse JSON.");
        return;
    }

    // Memperbarui nilai key yang diberikan
    doc[key] = newValue; // Memperbarui nilai untuk key tertentu

    // Serialize JSON kembali ke dalam String
    String updatedJson;
    serializeJsonPretty(doc, updatedJson);

    // Menyimpan JSON yang telah diperbarui ke dalam file
    writeJSON(filename, updatedJson);

    // Menambahkan log
}

bool Storage::serveFile(const String &filename, AsyncWebServerRequest *request, const String &contentType, const String &contentEncoding)
{
    String fullPath = filename.startsWith("/") ? filename : "/" + filename;

    if (!LittleFS.exists(fullPath))
    {
        Serial.println("File " + fullPath + " not found.");
        return false;
    }

    // Kirim file dengan header yang sesuai
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, fullPath, contentType);
    if (!contentEncoding.isEmpty())
    {
        response->addHeader("Content-Encoding", contentEncoding);
    }
    request->send(response);
    return true;
}