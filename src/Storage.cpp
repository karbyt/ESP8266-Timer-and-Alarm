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
        String defaultSong = "{}";
        writeJSON("/songs.json", defaultSong);
        Serial.println("Default audio.json created.");
    }
    if (!LittleFS.exists("/alarm.json"))
    {
        Serial.println("songs.json not found. Creating default...");
        String defaultAlarm = "{}";
        writeJSON("/alarm.json", defaultAlarm);
        Serial.println("Default alarm.json created.");
    }
    if (!LittleFS.exists("/relay.json"))
    {
        Serial.println("relay.json not found. Creating default...");
        String defaultRelay = "{\"relay1\":\"off\",\"relay2\":\"off\"}";
        writeJSON("/relay.json", defaultRelay);
        Serial.println("Default relay.json created.");
    }
    if (!LittleFS.exists("/timer.json"))
    {
        Serial.println("timer.json not found. Creating default...");
        String defaultTimer = "{}";
        writeJSON("/timer.json", defaultTimer);
        Serial.println("Default timer.json created.");
    }
    if (!LittleFS.exists("/timerstate.json"))
    {
        Serial.println("timerstate.json not found. Creating default...");
        String defaultTimerState = "{}";
        writeJSON("/timerstate.json", defaultTimerState);
        Serial.println("Default timerstate.json created.");
    }
}

String Storage::readJSON(const String &filename)
{
    if (!LittleFS.exists(filename))
    {
        Serial.println("File " + filename + " does not exist.");
        // Kembalikan JSON kosong atau default jika file tidak ada
        return "{}";
    }

    File file = LittleFS.open(filename, "r");
    if (!file)
    {
        Serial.println("Failed to open file " + filename + " for reading");
        return "{}"; // Kembalikan JSON kosong jika file gagal dibuka
    }

    String jsonContent;
    while (file.available())
    {
        jsonContent += char(file.read());
    }
    file.close();

    return jsonContent;
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
    serializeJson(doc, updatedJson);

    // Menyimpan JSON yang telah diperbarui ke dalam file
    writeJSON(filename, updatedJson);

    // Menambahkan log
}
