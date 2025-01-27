// AlarmManager.cpp
#include "AlarmManager.h"
#include "Storage.h"

AlarmManager alarmManager;

void AlarmManager::init(TimeManager *timeManager)
{
    _timeManager = timeManager;
}

void AlarmManager::checkAlarms()
{
    if (!_timeManager->isTimeSet())
        return;

    String alarmsJson = storage.readJSON("/alarm.json");
    JsonDocument doc; // Pastikan buffer cukup besar
    DeserializationError error = deserializeJson(doc, alarmsJson);

    if (error)
        return;

    JsonArray alarms = doc.as<JsonArray>();

    int currentHour = _timeManager->getHours();
    int currentMinute = _timeManager->getMinutes();
    int currentDay = _timeManager->getDay(); // Misalnya, Minggu = 0, Senin = 1, dst.

    static bool alarmTriggeredThisMinute = false;

    // Reset flag setelah detik 0 berlalu
    if (_timeManager->getSeconds() > 0)
    {
        alarmTriggeredThisMinute = false;
    }

    if (!alarmTriggeredThisMinute)
    {
        for (JsonVariant alarmJson : alarms)
        {
            String timeStr = alarmJson["time"].as<String>();
            int alarmHour = timeStr.substring(0, 2).toInt();
            int alarmMinute = timeStr.substring(3).toInt();

            // Pastikan `days` diubah ke integer jika itu string
            String daysStr = alarmJson["days"].as<String>();
            // Balikkan string `days` agar sesuai dengan format bitmask
            std::reverse(daysStr.begin(), daysStr.end());
            int days = strtol(daysStr.c_str(), nullptr, 2); // Konversi dari string biner ke integer

            // Periksa apakah alarm aktif pada hari ini
            bool isDayActive = (days & (1 << currentDay));

            if (currentHour == alarmHour &&
                currentMinute == alarmMinute &&
                _timeManager->getSeconds() == 0 &&
                isDayActive)
            {
                // Debug logging
                Serial.print("Day active check: ");
                Serial.print("Current day: ");
                Serial.print(currentDay);
                Serial.print(", Days value: ");
                Serial.print(days, BIN); // Cetak dalam format biner untuk debug
                Serial.print(", Bitwise check: ");
                Serial.println(isDayActive ? "TRUE" : "FALSE");

                // Ekstrak parameter alarm
                int ringtone = alarmJson["ringtone"].as<int>();
                String relay = alarmJson["relay"].as<String>();
                int relayDuration = alarmJson["relay_duration"].as<int>();

                // Trigger the alarm
                buzzer.beep(1);
                audio.play(ringtone);

                Serial.println("========ALARM TRIGGERED=========");

                alarmTriggeredThisMinute = true; // Mencegah retrigger dalam menit yang sama
                break;
            }
        }
    }
}

void AlarmManager::addAlarm(const String &label, const String &time, String days, int ringtone, const String &relay, int relayDuration)
{
    String alarmsJson = storage.readJSON("/alarm.json");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, alarmsJson);

    JsonArray alarms;
    if (error || doc.isNull() || (doc.is<JsonObject>() && doc.size() == 0) || (doc.is<JsonArray>() && doc.size() == 0))
    {
        // If JSON is invalid or null, create a new array
        alarms = doc.to<JsonArray>();
    }
    else
    {
        alarms = doc.as<JsonArray>();
    }

    int nextId = 1;
    for (JsonVariant alarmJson : alarms)
    {
        int currentId = alarmJson["id"].as<int>();
        if (currentId >= nextId)
        {
            nextId = currentId + 1;
        }
    }

    JsonObject newAlarm = alarms.createNestedObject();
    newAlarm["id"] = nextId;
    newAlarm["label"] = label;
    newAlarm["time"] = time;
    newAlarm["days"] = days;
    newAlarm["ringtone"] = ringtone;
    newAlarm["relay"] = relay;
    newAlarm["relay_duration"] = relayDuration;

    String updatedJson;
    serializeJsonPretty(doc, updatedJson);
    storage.writeJSON("/alarm.json", updatedJson);
    Serial.println("Alarm added successfully teko alarmmanager" + updatedJson);
}

void AlarmManager::removeAlarm(int id)
{
    String alarmsJson = storage.readJSON("/alarm.json");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, alarmsJson);

    if (error)
    {
        Serial.println("Failed to parse alarm JSON");
        return;
    }

    JsonArray alarms = doc.as<JsonArray>();

    for (size_t i = 0; i < alarms.size(); i++)
    {
        if (alarms[i]["id"].as<int>() == id)
        {
            alarms.remove(i);
            break;
        }
    }

    int newId = 1;
    for (JsonVariant alarmJson : alarms)
    {
        alarmJson["id"] = newId++;
    }

    String updatedJson;
    serializeJsonPretty(doc, updatedJson);
    storage.writeJSON("/alarm.json", updatedJson);
}

bool AlarmManager::updateAlarm(int id, String label, String time, String days, int ringtone, String relay, int relayDuration)
{
    String alarmsJson = storage.readJSON("/alarm.json");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, alarmsJson);

    if (error)
    {
        return false;
    }

    JsonArray alarms = doc.as<JsonArray>();

    for (JsonObject alarm : alarms)
    {
        if (alarm["id"] == id)
        {
            // Update the alarm properties
            alarm["label"] = label;
            alarm["time"] = time;
            alarm["days"] = days; // Store days as string to preserve leading zero
            alarm["ringtone"] = ringtone;
            alarm["relay"] = relay;
            alarm["relay_duration"] = relayDuration;

            // Save the updated alarms back to the file
            String updatedAlarmsJson;
            serializeJsonPretty(doc, updatedAlarmsJson);
            storage.writeJSON("/alarm.json", updatedAlarmsJson);

            return true;
        }
    }

    return false; // Alarm with the specified ID was not found
}

String AlarmManager::getAlarmsJson()
{
    return storage.readJSON("/alarm.json");
}